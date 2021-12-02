/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "time_internal.h"
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <securec.h>
#include "los_config.h"
#include "los_task.h"
#include <los_swtmr.h>
#include "los_atomic.h"
#include "los_event.h"
/* begin jbc 2021-11-25 */
#include "los_mux.h"
/* ebd jbc 2021-11-25 */

typedef struct {
    volatile INT32 *realValue;
    INT32 value;
    UINT32 clearEvent;
} EventCond;

#define INLINE inline

#define BROADCAST_EVENT     1
#define COND_COUNTER_STEP   0x0004U
#define COND_FLAGS_MASK     0x0003U
#define COND_COUNTER_MASK   (~COND_FLAGS_MASK)
/* begin jbc 2021-11-25 */
#define INT_MAX  0x7fffffff

/*
 * Check whether there is a cancel pending and if so, whether
 * cancellations are enabled. We do it in this order to reduce the
 * number of tests in the common case - when no cancellations are
 * pending. We make this inline so it can be called directly below for speed
 */
STATIC INT32 CheckForCancel(VOID)
{
    LosTaskCB *tcb = OS_TCB_FROM_TID(LOS_CurTaskIDGet());
    // if (self->canceled && (self->cancelstate == PTHREAD_CANCEL_ENABLE)) {
    if ((tcb->canceled) && (tcb->cancelstate == PTHREAD_CANCEL_ENABLE)) {
        return 1;
    }
    return 0;
}

int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *shared)
{
    if ((attr == NULL) || (shared == NULL)) {
        return EINVAL;
    }

    *shared = PTHREAD_PROCESS_PRIVATE;

    return 0;
}

int pthread_condattr_setpshared(pthread_condattr_t *attr, int shared)
{
    (VOID)attr;
    if ((shared != PTHREAD_PROCESS_PRIVATE) && (shared != PTHREAD_PROCESS_SHARED)) {
        return EINVAL;
    }

    if (shared != PTHREAD_PROCESS_PRIVATE) {
        return ENOSYS;
    }

    return 0;
}
/* end jbc 2021-11-25 */

int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }

    (VOID)memset_s(attr, sizeof(pthread_condattr_t), 0, sizeof(pthread_condattr_t));
	/* begin jbc 2021-11-25 */
    attr->clock = INT_MAX;
	/* end jbc 2021-11-25 */
    return 0;
}
/* begin jbc 2021-11-25 */
int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock)
{
    if ((attr == NULL) || (clock == NULL)) {
        return -1;
    }

    *clock = attr->clock;
    return 0;
}
/* end jbc 2021-11-25 */

int pthread_condattr_init(pthread_condattr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }

    attr->clock = CLOCK_REALTIME;
    return 0;
}

int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clk)
{
    if ((attr == NULL) || (clk < 0)) {
        return EINVAL;
    }

    if ((clk != CLOCK_REALTIME) && (clk != CLOCK_MONOTONIC) &&
        (clk != CLOCK_PROCESS_CPUTIME_ID) && (clk != CLOCK_THREAD_CPUTIME_ID)) {
        return EINVAL;
    }

    attr->clock = clk;
    return 0;
}

STATIC INLINE INT32 CondInitCheck(const pthread_cond_t *cond)
{
    if ((cond->event.stEventList.pstPrev == NULL) &&
        (cond->event.stEventList.pstNext == NULL)) {
        return 1;
    }
    return 0;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    int ret = ENOERR;
    pthread_condattr_t condAttr;

    if (cond == NULL) {
        return EINVAL;
    }

    if (attr == NULL) {
        pthread_condattr_init(&condAttr);
        attr = &condAttr;
    }

    (VOID)LOS_EventInit(&(cond->event));

    cond->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (cond->mutex == NULL) {
        return ENOMEM;
    }

    (VOID)pthread_mutex_init(cond->mutex, NULL);

    cond->value = 0;
    (VOID)pthread_mutex_lock(cond->mutex);
    cond->count = 0;
    cond->clock = attr->clock;
    (VOID)pthread_mutex_unlock(cond->mutex);

    return ret;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
    if (cond == NULL) {
        return EINVAL;
    }

    if (CondInitCheck(cond)) {
        return ENOERR;
    }

    if (LOS_EventDestroy(&cond->event) != LOS_OK) {
        return EBUSY;
    }
    if (pthread_mutex_destroy(cond->mutex) != ENOERR) {
        PRINT_ERR("%s mutex destroy fail!\n", __FUNCTION__);
        return EINVAL;
    }
    free(cond->mutex);
    cond->mutex = NULL;
    return ENOERR;
}

STATIC VOID PthreadCountSub(pthread_cond_t *cond)
{
    (VOID)pthread_mutex_lock(cond->mutex);
    if (cond->count > 0) {
        cond->count--;
    }
    (VOID)pthread_mutex_unlock(cond->mutex);
}


int pthread_cond_broadcast(pthread_cond_t *cond)
{
    int ret = ENOERR;

    if (cond == NULL) {
        return EINVAL;
    }

    (VOID)pthread_mutex_lock(cond->mutex);
    if (cond->count > 0) {
        cond->count = 0;
        (VOID)pthread_mutex_unlock(cond->mutex);
        (VOID)LOS_EventWrite(&(cond->event), BROADCAST_EVENT);
        return ret;
    }
    (VOID)pthread_mutex_unlock(cond->mutex);

    return ret;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    int ret = ENOERR;

    if (cond == NULL) {
        return EINVAL;
    }

    (VOID)pthread_mutex_lock(cond->mutex);
    if (cond->count > 0) {
        cond->count--;
        (VOID)pthread_mutex_unlock(cond->mutex);
        // This should modify to once.
        (VOID)LOS_EventWrite(&(cond->event), BROADCAST_EVENT);

        return ret;
    }
    (VOID)pthread_mutex_unlock(cond->mutex);

    return ret;
}

STATIC INT32 ProcessReturnVal(pthread_cond_t *cond, INT32 val)
{
    INT32 ret;
    switch (val) {
        /* 0: event does not occur */
        case 0:
        case BROADCAST_EVENT:
            ret = ENOERR;
            break;
        case LOS_ERRNO_EVENT_READ_TIMEOUT:
            PthreadCountSub(cond);
            ret = ETIMEDOUT;
            break;
        default:
            PthreadCountSub(cond);
            ret = EINVAL;
            break;
    }
    return ret;
}

/* begin jbc 2021-11-25 */
/*
 * Test for a pending cancellation for the current thread and terminate
 * the thread if there is one.
 */
void pthread_testcancel(void)
{
    if (CheckForCancel()) {
        /*
         * If we have cancellation enabled, and there is a cancellation
         * pending, then go ahead and do the deed.
         * Exit now with special retVal. pthread_exit() calls the
         * cancellation handlers implicitly.
         */
        pthread_exit((void *)PTHREAD_CANCELED);
    }
}
/* end jbc 2021-11-25 */

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec *ts)
{
    INT32 ret;
    UINT32 absTicks;
    const UINT32 nsPerTick = OS_SYS_NS_PER_SECOND / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    struct timespec tp;
    UINT64 nseconds;
    UINT64 currTime;
    
    /* begin jbc 2021-11-25 */
    LosMuxCB *muxPosted = NULL;
    pthread_testcancel();
    
    if ((cond == NULL) || (mutex == NULL) || (ts == NULL) || (mutex->magic != _MUX_MAGIC)) {
        return EINVAL;
    }

    muxPosted = GET_MUX(mutex->handle);
    if ((mutex->stAttr.type == PTHREAD_MUTEX_ERRORCHECK) && (g_losTask.runTask != muxPosted->owner)) {
         return EPERM;
     }
    /* end jbc 2021-11-25 */
    if (CondInitCheck(cond)) {
        ret = pthread_cond_init(cond, NULL);
        if (ret != ENOERR) {
            return ret;
        }
    }

    (VOID)pthread_mutex_lock(cond->mutex);
    cond->count++;
    (VOID)pthread_mutex_unlock(cond->mutex);

    if (!ValidTimeSpec(ts)) {
        return EINVAL;
    }

    clock_gettime(cond->clock, &tp);
    currTime = (UINT64)tp.tv_sec * OS_SYS_NS_PER_SECOND + tp.tv_nsec;
    nseconds = (UINT64)ts->tv_sec * OS_SYS_NS_PER_SECOND + ts->tv_nsec;
    if (currTime >= nseconds) {
        return ETIMEDOUT;
    }
    absTicks = ((nseconds - currTime) + nsPerTick - 1) / nsPerTick + 1;
    if (absTicks >= UINT32_MAX) {
        return EINVAL;
    }

    if (pthread_mutex_unlock(mutex) != ENOERR) {
        PRINT_ERR("%s: %d failed\n", __FUNCTION__, __LINE__);
    }

    ret = (INT32)LOS_EventRead(&(cond->event), 0x0f, LOS_WAITMODE_OR | LOS_WAITMODE_CLR, absTicks);

    if (pthread_mutex_lock(mutex) != ENOERR) {
        PRINT_ERR("%s: %d failed\n", __FUNCTION__, __LINE__);
    }

    ret = ProcessReturnVal(cond, ret);
    /* begin jbc 2021-11-25 */
    pthread_testcancel();
    /* end jbc 2021-11-25 */
    return ret;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    int ret;

    if ((cond == NULL) || (mutex == NULL)) {
        return EINVAL;
    }

    if (CondInitCheck(cond)) {
        ret = pthread_cond_init(cond, NULL);
        if (ret != ENOERR) {
            return ret;
        }
    }

    (VOID)pthread_mutex_lock(cond->mutex);
    cond->count++;
    (VOID)pthread_mutex_unlock(cond->mutex);

    if (pthread_mutex_unlock(mutex) != ENOERR) {
        PRINT_ERR("%s: %d failed\n", __FUNCTION__, __LINE__);
    }
    ret = (INT32)LOS_EventRead(&(cond->event), 0x0f, LOS_WAITMODE_OR | LOS_WAITMODE_CLR, LOS_WAIT_FOREVER);
    if (pthread_mutex_lock(mutex) != ENOERR) {
        PRINT_ERR("%s: %d failed\n", __FUNCTION__, __LINE__);
    }

    switch (ret) {
        /* 0: event does not occur */
        case 0:
        case BROADCAST_EVENT:
            ret = ENOERR;
            break;
        default:
            PthreadCountSub(cond);
            ret = EINVAL;
            break;
    }

    return ret;
}

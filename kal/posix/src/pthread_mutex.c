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

#include <pthread.h>
#include <time.h>
#include <securec.h>
#include "los_compiler.h"
#include "los_mux.h"
#include "errno.h"

/* begin jbc 2021-11-25 */
#include "los_mux.h"
#define MUTEXATTR_TYPE_MASK 0x0fU
/* end jbc 2021-11-25 */
#define OS_SYS_NS_PER_MSECOND 1000000
#define OS_SYS_NS_PER_SECOND  1000000000

static inline int MapError(UINT32 err)
{
    switch (err) {
        case LOS_OK:
            return 0;
        case LOS_ERRNO_MUX_PEND_INTERR:
            return EPERM;
        case LOS_ERRNO_MUX_PEND_IN_LOCK:
            return EDEADLK;
        case LOS_ERRNO_MUX_PENDED:
        case LOS_ERRNO_MUX_UNAVAILABLE:
            return EBUSY;
        case LOS_ERRNO_MUX_TIMEOUT:
            return ETIMEDOUT;
        case LOS_ERRNO_MUX_ALL_BUSY:
            return EAGAIN;
        case LOS_ERRNO_MUX_INVALID:
        default:
            return EINVAL;
    }
}

int pthread_mutexattr_init(pthread_mutexattr_t *mutexAttr)
{
    if (mutexAttr == NULL) {
        return EINVAL;
    }
    /* begin jbc 2021-11-25 */
    mutexAttr->type = PTHREAD_MUTEX_DEFAULT;
    /* ebd jbc 2021-11-25 */
    return 0;
}

/* begin jbc 2021-11-25 */
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *outType)
{
    INT32 type;

    if ((attr == NULL) || (outType == NULL)) {
        return EINVAL;
    }

    type = (INT32)(attr->type & MUTEXATTR_TYPE_MASK);
    if ((type < PTHREAD_MUTEX_NORMAL) || (type > PTHREAD_MUTEX_ERRORCHECK)) {
        return EINVAL;
    }

    *outType = type;
    return ENOERR;
}
/* end jbc 2021-11-25 */

int pthread_mutexattr_settype(pthread_mutexattr_t *mutexAttr, int type)
{
    /* begin jbc 2021-11-25 */
    if ((mutexAttr == NULL) ||
        (type < PTHREAD_MUTEX_NORMAL) ||
        (type > PTHREAD_MUTEX_ERRORCHECK)) {
        return EINVAL;
    }
    mutexAttr->type = (UINT8)((mutexAttr->type & ~MUTEXATTR_TYPE_MASK) | (UINT32)type);
	/* end jbc 2021-11-25 */
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *mutexAttr)
{
    if (mutexAttr == NULL) {
        return EINVAL;
    }

    (VOID)memset_s(mutexAttr, sizeof(pthread_mutexattr_t), 0, sizeof(pthread_mutexattr_t));
    return 0;
}

/* Initialize mutex. If mutexAttr is NULL, use default attributes. */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexAttr)
{
    /* begin jbc 2021-11-25 */
    pthread_mutexattr_t useAttr;
    /* end jbc 2021-11-25 */
    UINT32 muxHandle;
    UINT32 ret;

    /* begin jbc 2021-11-25 */
    if (mutex == NULL) {
        return EINVAL;
    }
    
    if (mutexAttr == NULL){
        (VOID)pthread_mutexattr_init(&useAttr);
    } else {
        useAttr = *mutexAttr;
    }
    /* end jbc 2021-11-25 */

    ret = LOS_MuxCreate(&muxHandle);
    if (ret != LOS_OK) {
        return MapError(ret);
    }
    /* begin jbc 2021-11-25 */
    mutex->stAttr = useAttr;
    /* end jbc 2021-11-25 */
    mutex->magic = _MUX_MAGIC;
    mutex->handle = muxHandle;

    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    UINT32 ret;
    /* begin jbc 2021-11-25 */
    if ((mutex == NULL) || (mutex->magic != _MUX_MAGIC)) {
        return EINVAL;
    }
    /* end jbc 2021-11-25 */
    ret = LOS_MuxDelete(mutex->handle);
    if (ret != LOS_OK) {
        return MapError(ret);
    }
    mutex->handle = _MUX_INVALID_HANDLE;
    mutex->magic = 0;
    return 0;
}
/* begin jbc 2021-11-25 */
STATIC UINT32 CheckMutexAttr(const pthread_mutexattr_t *attr)
{
    if (((INT8)(attr->type) < PTHREAD_MUTEX_NORMAL) ||
        (attr->type > PTHREAD_MUTEX_ERRORCHECK)) {
        return LOS_NOK;
    }
    return LOS_OK;
}

UINT32 OsMuxPreCheck(const pthread_mutex_t *mutex, const LosTaskCB *runTask)
{
    if (CheckMutexAttr(&mutex->stAttr) != LOS_OK) {
        return EINVAL;
    }

    return ENOERR;
}
/* end jbc 2021-11-25 */
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *absTimeout)
{
    UINT32 ret;
    UINT32 timeout;
    UINT64 timeoutNs;
    struct timespec curTime = {0};
    /* begin jbc 2021-11-25 */
    LosMuxCB *muxPended = NULL;
    
    if ((mutex == NULL) || (absTimeout == NULL) || (mutex->magic != _MUX_MAGIC) || (absTimeout->tv_nsec < 0) || (absTimeout->tv_nsec >= OS_SYS_NS_PER_SECOND)) {
    /* end jbc 2021-11-25 */
        return EINVAL;
    }
    if (mutex->handle == _MUX_INVALID_HANDLE) {
        ret = LOS_MuxCreate(&mutex->handle);
        if (ret != LOS_OK) {
            return MapError(ret);
        }
    /* begin jbc 2021-11-25 */
    } else {
        muxPended = GET_MUX(mutex->handle);
        if (((INT8)(mutex->stAttr.type) < PTHREAD_MUTEX_NORMAL) ||
            (mutex->stAttr.type > PTHREAD_MUTEX_ERRORCHECK)) {
            return EINVAL;
        }
        //ret = OsMuxPreCheck(mutex, &g_losTask.runTask);
        // if (ret != ENOERR) {
        //     return (INT32)ret;
        // }

        if ((mutex->stAttr.type == PTHREAD_MUTEX_ERRORCHECK) &&
            (muxPended->muxCount != 0) &&
            (muxPended->owner == OS_TCB_FROM_TID(LOS_CurTaskIDGet()))) {
            return EDEADLK;
        } 
    }
    /* end jbc 2021-11-25 */

    ret = clock_gettime(CLOCK_REALTIME, &curTime);
    if (ret != LOS_OK) {
        return EINVAL;
    }
    timeoutNs = (absTimeout->tv_sec - curTime.tv_sec) * OS_SYS_NS_PER_SECOND + (absTimeout->tv_nsec - curTime.tv_nsec);
    if (timeoutNs <= 0) {
        return ETIMEDOUT;
    }
    timeout = (timeoutNs + (OS_SYS_NS_PER_MSECOND - 1)) / OS_SYS_NS_PER_MSECOND;
    ret = LOS_MuxPend(mutex->handle, timeout);
    return MapError(ret);
}

/* Lock mutex, waiting for it if necessary. */
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    UINT32 ret;
    /* begin jbc 2021-11-25 */
    LosMuxCB *muxPended = NULL;
    if ((mutex == NULL ) || (mutex->magic != _MUX_MAGIC) || ((INT8)(mutex->stAttr.type) < PTHREAD_MUTEX_NORMAL) ||
        (mutex->stAttr.type > PTHREAD_MUTEX_ERRORCHECK)) {
        return EINVAL;
    }
    /* end jbc 2021-11-25 */
    if (mutex->handle == _MUX_INVALID_HANDLE) {
        ret = LOS_MuxCreate(&mutex->handle);
        if (ret != LOS_OK) {
            return MapError(ret);
        }
    /* begin jbc 2021-11-25 */
    } else {
        muxPended = GET_MUX(mutex->handle);
        if ((mutex->stAttr.type == PTHREAD_MUTEX_ERRORCHECK) &&
            (muxPended->muxCount != 0) &&
            (muxPended->owner == OS_TCB_FROM_TID(LOS_CurTaskIDGet()))) {
            return EDEADLK;
        } 
    }
    /* end jbc 2021-11-25 */
    ret = LOS_MuxPend(mutex->handle, LOS_WAIT_FOREVER);
    return MapError(ret);
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    UINT32 ret;
    /* begin jbc 2021-11-25 */
    LosMuxCB *muxPended = NULL;
    if ((mutex == NULL) || (mutex->magic != _MUX_MAGIC)) {
        return EINVAL;
    }
    /* end jbc 2021-11-25 */
    if (mutex->handle == _MUX_INVALID_HANDLE) {
        ret = LOS_MuxCreate(&mutex->handle);
        if (ret != LOS_OK) {
            return MapError(ret);
        }
    /* begin jbc 2021-11-25 */
    } else {
        muxPended = GET_MUX(mutex->handle);
        if ((muxPended->owner != NULL) && (muxPended->owner != OS_TCB_FROM_TID(LOS_CurTaskIDGet()))) {
            return EBUSY;
        }
        if ((mutex->stAttr.type != PTHREAD_MUTEX_RECURSIVE) && (muxPended->muxCount != 0)) {
            return EBUSY;
        }
    }
    /* end jbc 2021-11-25 */
    ret = LOS_MuxPend(mutex->handle, 0);
    return MapError(ret);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    UINT32 ret;
    /* begin jbc 2021-11-25 */
    if ((mutex == NULL) || (mutex->magic != _MUX_MAGIC) || ((INT8)(mutex->stAttr.type) < PTHREAD_MUTEX_NORMAL) ||
        (mutex->stAttr.type > PTHREAD_MUTEX_ERRORCHECK)) {
        return EINVAL;
    }
    /* begin jbc 2021-11-25 */
    ret = LOS_MuxPost(mutex->handle);
    return MapError(ret);
}


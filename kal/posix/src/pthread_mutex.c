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
#include "los_task.h"
#include "los_sched.h"
#include "errno.h"


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

int pthread_mutexattr_check(pthread_mutexattr_t *mutexAttr)
{
    if (mutexAttr == NULL) {
        return EINVAL;
    }

    if (mutexAttr->type != PTHREAD_MUTEX_NORMAL &&
        mutexAttr->type != PTHREAD_MUTEX_RECURSIVE &&
        mutexAttr->type != PTHREAD_MUTEX_ERRORCHECK) {
            return EINVAL;
    }
    return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *mutexAttr)
{
    if (mutexAttr == NULL) {
        return EINVAL;
    }

    mutexAttr->type = PTHREAD_MUTEX_NORMAL;
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *mutexAttr, int type)
{
    if (pthread_mutexattr_check(mutexAttr)) {
        return EINVAL;
    }

    if (type != PTHREAD_MUTEX_NORMAL &&
        type != PTHREAD_MUTEX_RECURSIVE &&
        type != PTHREAD_MUTEX_ERRORCHECK) {
            return EINVAL;
    }

    mutexAttr->type = (unsigned)type;
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
    UINT32 intSave;

    if (mutex == NULL) {
        return EINVAL;
    }
 
    if (mutexAttr == NULL) {
        if (pthread_mutexattr_init(&mutex->attr) != 0) {
            return EINVAL;
        }
    } else {
        (VOID)memcpy_s(&mutex->attr, sizeof(pthread_mutexattr_t), mutexAttr, sizeof(pthread_mutexattr_t));
    }

    if (pthread_mutexattr_check(&mutex->attr) != 0) {
        return EINVAL;
    }

    intSave = LOS_IntLock();
    mutex->muxCount = 0;
    mutex->owner = NULL;
    mutex->magic = _MUX_MAGIC;
    LOS_ListInit(&mutex->muxList);
    LOS_IntRestore(intSave);
    return 0;
}

static int pthread_mutex_pend(pthread_mutex_t *mutex, UINT32 timeout)
{
    UINT32 intSave;

    LosTaskCB *runningTask = g_losTask.runTask;

    if (pthread_mutexattr_check(&mutex->attr) != 0) {
        return EINVAL;
    }
    if ((mutex->muxList.pstNext == NULL) || (mutex->muxList.pstPrev == NULL)) {
        LOS_ListInit(&mutex->muxList);
    }

    if ((mutex->attr.type == PTHREAD_MUTEX_ERRORCHECK) && (mutex->owner == (VOID *)runningTask)) {
        return EDEADLK;
    }

    if (runningTask->taskStatus & OS_TASK_FLAG_SYSTEM_TASK) {
        return LOS_ERRNO_MUX_PEND_IN_SYSTEM_TASK;
    }

    intSave = LOS_IntLock();
    if (mutex->muxCount == 0) {
        mutex->muxCount++;
        mutex->owner = (VOID *)runningTask;
        mutex->priority = runningTask->priority;
        LOS_IntRestore(intSave);
        return 0;
    }

    if ((mutex->attr.type == PTHREAD_MUTEX_RECURSIVE) && (mutex->owner == (VOID *)runningTask)) {
        mutex->muxCount++;
        LOS_IntRestore(intSave);
        return 0;
    }

    if (!timeout) {
        LOS_IntRestore(intSave);
        return EINVAL;
    }

    if (g_losTaskLock != 0) {
        LOS_IntRestore(intSave);
        return EDEADLK;
    }
    LOS_IntRestore(intSave);

    /* temporary adjust priority for mutex owner */
    if (((LosTaskCB *)(mutex->owner))->priority > runningTask->priority) {
        if (LOS_TaskPriSet(((LosTaskCB *)(mutex->owner))->taskID, runningTask->priority) != 0) {
            return EINVAL;
        }
    }

    intSave = LOS_IntLock();
    OsSchedTaskWait(&mutex->muxList, timeout);
    LOS_IntRestore(intSave);

    LOS_Schedule();

    intSave = LOS_IntLock();
    if (runningTask->taskStatus & OS_TASK_STATUS_TIMEOUT) {
        runningTask->taskStatus &= (~OS_TASK_STATUS_TIMEOUT);
        return ETIMEDOUT;
    }
    LOS_IntRestore(intSave);
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    UINT32 intSave;

    if (mutex == NULL) {
        return EINVAL;
    }
    if (mutex->magic != _MUX_MAGIC) {
        return EINVAL;
    }
    if (mutex->muxCount != 0) {
        return EBUSY;
    }

    intSave = LOS_IntLock();
    (VOID)memset_s(mutex, sizeof(pthread_mutex_t), 0, sizeof(pthread_mutex_t));
    LOS_IntRestore(intSave);
    return 0;
}

int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *absTimeout)
{
    INT32 ret;
    UINT32 timeout;
    UINT64 timeoutNs;
    struct timespec curTime = {0};

    if (mutex == NULL) {
        return EINVAL;
    }
    if ((mutex->magic != _MUX_MAGIC) || (absTimeout->tv_nsec < 0) || (absTimeout->tv_nsec >= OS_SYS_NS_PER_SECOND)) {
        return EINVAL;
    }

    if (clock_gettime(CLOCK_REALTIME, &curTime) != 0) {
        return EINVAL;
    }
    timeoutNs = (absTimeout->tv_sec - curTime.tv_sec) * OS_SYS_NS_PER_SECOND + (absTimeout->tv_nsec - curTime.tv_nsec);
    if (timeoutNs <= 0) {
        return ETIMEDOUT;
    }
    timeout = (timeoutNs + OS_SYS_NS_PER_MSECOND - 1) / OS_SYS_NS_PER_MSECOND;

    ret = pthread_mutex_pend(mutex, timeout);
    return ret;
}

/* Lock mutex, waiting for it if necessary. */
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    INT32 ret;

    if (mutex == NULL) {
        return EINVAL;
    }
    if (mutex->magic != _MUX_MAGIC) {
        return EBADF;
    }

    ret = pthread_mutex_pend(mutex, LOS_WAIT_FOREVER);
    return ret;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    INT32 ret;

    if (mutex == NULL) {
        return EINVAL;
    }
    if (mutex->magic != _MUX_MAGIC) {
        return EINVAL;
    }
    if (mutex->owner != NULL && (LosTaskCB *)mutex->owner != g_losTask.runTask) {
        return EBUSY;
    } else if (mutex->muxCount != 0 && mutex->attr.type != PTHREAD_MUTEX_RECURSIVE) {
        return EBUSY;
    }

    ret = pthread_mutex_pend(mutex, 0);
    return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    UINT32 intSave;
    LosTaskCB *resumedTask = NULL;

    if (mutex == NULL) {
        return EINVAL;
    }
    if (mutex->magic != _MUX_MAGIC) {
        return EBADF;
    }
    if (pthread_mutexattr_check(&mutex->attr) != 0) {
        return EINVAL;
    }
    if ((LosTaskCB *)mutex->owner != g_losTask.runTask) {
        return EPERM;
    }
    if (mutex->muxCount == 0) {
        return EPERM;
    }

    intSave = LOS_IntLock();
    if ((mutex->muxList.pstNext == NULL) || (mutex->muxList.pstPrev == NULL)) {
        LOS_ListInit(&mutex->muxList);
    }
    LOS_IntRestore(intSave);

    /* restore mutex priority */
    if (((LosTaskCB *)mutex->owner)->priority != mutex->priority) {
        if (LOS_CurTaskPriSet(mutex->priority) != 0) {
            return EPERM;
        }
    }
    if ((--mutex->muxCount != 0) && (mutex->attr.type == PTHREAD_MUTEX_RECURSIVE)) {
        return 0;
    }

    intSave = LOS_IntLock();
    if (!LOS_ListEmpty(&mutex->muxList)) {
        resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&(mutex->muxList)));
        mutex->muxCount = 1;
        mutex->owner = resumedTask;
        mutex->priority = resumedTask->priority;
        OsSchedTaskWake(resumedTask);
        LOS_IntRestore(intSave);

        LOS_Schedule();
    } else {
        mutex->owner = NULL;
        LOS_IntRestore(intSave);
    }
    return 0;
}

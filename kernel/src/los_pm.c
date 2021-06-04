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

#include "los_pm.h"
#include "securec.h"
#include "los_sched.h"
#include "los_timer.h"
#include "los_memory.h"

#if (LOSCFG_KERNEL_PM == 1)
#define OS_PM_NODE_FREE 0x80000000U
#define OS_PM_LOCK_MAX  0xFFFFU

#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
STATIC UINT64 g_enterSleepTime;
#endif

#if (LOSCFG_KERNEL_PM_DEBUG == 1)
typedef struct {
    CHAR         *name;
    UINT32       count;
    LOS_DL_LIST  list;
} OsPmLockCB;
#endif

typedef struct {
    UINT16            pmMode;
    UINT16            mode;
    UINT16            lock;
    LOS_DL_LIST       head;         /* devices list */
    LosPmSysctrl      *sysctrl;
#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
    LosPmLowFreqTimer *lowFreqTimer;
#endif
#if (LOSCFG_KERNEL_PM_DEBUG == 1)
    LOS_DL_LIST       lockList;
#endif
} LosPmCB;

STATIC LosPmCB g_pmCB;

#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
STATIC VOID OsPmTickTimerStart(LosPmLowFreqTimer *lowFreqTimer)
{
    UINT32 intSave;
    UINT64 oldTime;
    UINT64 currTime, sleepTime, realSleepTime;

    intSave = LOS_IntLock();
    /* Restore the main CPU frequency */
    sleepTime = lowFreqTimer->getTimeCycle();
    lowFreqTimer->stop(lowFreqTimer->arg);

    realSleepTime = OS_SYS_CYCLE_TO_NS(sleepTime, lowFreqTimer->freq);
    realSleepTime = OS_SYS_NS_TO_CYCLE(realSleepTime, OS_SYS_CLOCK);
    currTime = g_enterSleepTime + realSleepTime;
    g_enterSleepTime = 0;

    oldTime = HalGetTickCycle(NULL);
    LOS_ASSERT(currTime > oldTime);
    OsSchedTimerBaseReset(currTime - oldTime);
    OsSchedUpdateExpireTime(currTime);
    lowFreqTimer->unlock();
    LOS_IntRestore(intSave);
    return;
}

STATIC VOID OsPmTickTimerStop(LosPmLowFreqTimer *lowFreqTimer)
{
    UINT64 sleepCycle;
    UINT64 realSleepTime = OsSchedSleepTimeGet();

    if (realSleepTime == 0) {
        return;
    }

    sleepCycle = OS_SYS_CYCLE_TO_NS(realSleepTime, OS_SYS_CLOCK);
    sleepCycle = OS_SYS_NS_TO_CYCLE(sleepCycle, lowFreqTimer->freq);

    /* The main CPU reduces the frequency */
    g_enterSleepTime = OsGetCurrSchedTimeCycle();
    lowFreqTimer->lock();
    lowFreqTimer->start(lowFreqTimer->arg, sleepCycle);
    return;
}
#endif

STATIC VOID OsPmResumeDevice(LOS_DL_LIST *head, LOS_DL_LIST *tail, LOS_SysSleepEnum mode)
{
    LosPmDeviceOpt *opt = NULL;
    LOS_DL_LIST *list = head->pstNext;

    /* All devices are restored from low power mode */
    while (list != tail) {
        opt = LOS_DL_LIST_ENTRY(list, LosPmDeviceOpt, list);
        if (mode == LOS_SYS_DEEP_SLEEP && (opt->deepResume != NULL))  {
            opt->deepResume();
        } else if ((mode == LOS_SYS_SHUTDOWN) && (opt->shutdownResume != NULL)) {
            opt->shutdownResume();
        }

        list = list->pstNext;
    }

    return;
}

VOID OsPmRestore(VOID)
{
    UINT32 intSave;
    LosPmDeviceOpt *opt = NULL;
    LOS_DL_LIST *head = &g_pmCB.head;
    LOS_DL_LIST *list = head->pstNext;
    LosPmCB *pm = &g_pmCB;

    intSave = LOS_IntLock();
    /* Cpu is restored from low power mode */
    if ((pm->sysctrl != NULL) && (pm->sysctrl->resume != NULL)) {
        pm->sysctrl->resume(pm->mode);
    }

#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
    /* Sys tick timer is restored from low power mode */
    if ((pm->lowFreqTimer != NULL) && (g_enterSleepTime != 0)) {
        OsPmTickTimerStart(pm->lowFreqTimer);
    }
#endif

    OsPmResumeDevice(head, head, pm->mode);

    LOS_IntRestore(intSave);
    return;
}

VOID OsPmEnter(VOID)
{
    UINT32 ret;
    UINT32 intSave;
    PmSysctrlSuspend suspend = NULL;
    LosPmDeviceOpt *opt = NULL;
    LOS_DL_LIST *head = &g_pmCB.head;
    LOS_DL_LIST *list = head->pstNext;
    LosPmCB *pm = &g_pmCB;

    intSave = LOS_IntLock();

    pm->mode = pm->pmMode;
    if (pm->lock > 0) {
        pm->mode = LOS_SYS_NORMAL_SLEEP;
    }

    if (pm->mode != LOS_SYS_NORMAL_SLEEP) {
        /* All devices enter low power mode */
        while (list != head) {
            opt = LOS_DL_LIST_ENTRY(list, LosPmDeviceOpt, list);
            if ((pm->mode == LOS_SYS_DEEP_SLEEP) && (opt->deepSuspend != NULL)) {
                ret = opt->deepSuspend();
            } else if ((pm->mode == LOS_SYS_SHUTDOWN) && (opt->shutdownSuspend != NULL)) {
                ret = opt->shutdownSuspend();
            }

            if (ret != LOS_OK) {
                OsPmResumeDevice(head, list, pm->mode);
                pm->mode = LOS_SYS_NORMAL_SLEEP;
                break;
            }

            list = list->pstNext;
        }

#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
        /* Sys tick timer enter low power mode */
        if (pm->lowFreqTimer != NULL) {
            OsPmTickTimerStop(pm->lowFreqTimer);
        }
#endif
    }

    /* cpu enter low power mode */
    if (pm->sysctrl != NULL) {
        suspend = pm->sysctrl->suspend;
        LOS_IntRestore(intSave);
        suspend(pm->mode);
    } else {
        LOS_IntRestore(intSave);
    }

    return;
}

UINT32 LOS_PmRegistered(LosPmDeviceOpt *opt)
{
    UINT32 intSave;
    LosPmCB *pm = &g_pmCB;
    LosPmSysctrl *sysctrl = NULL;
#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
    LosPmLowFreqTimer *lowFreqTimer = NULL;
#endif

    if (opt == NULL) {
        return LOS_ERRNO_PM_INVALID_PARAM;
    }

    if (opt->flags & LOS_PM_FLAGS_LOW_POWER_TIMER) {
#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
        lowFreqTimer = (LosPmLowFreqTimer *)opt;
        if ((lowFreqTimer->start == NULL) || (lowFreqTimer->stop == NULL) ||
            (lowFreqTimer->getTimeCycle == NULL) || (lowFreqTimer->freq == 0) ||
            (lowFreqTimer->lock == NULL) || (lowFreqTimer->unlock == NULL)) {
            return LOS_ERRNO_PM_INVALID_PARAM;
        }

        g_enterSleepTime = 0;

        intSave = LOS_IntLock();
        pm->lowFreqTimer = lowFreqTimer;
        LOS_IntRestore(intSave);
        return LOS_OK;
#else
        return LOS_ERRNO_PM_INVALID_FLAG;
#endif
    } else if (opt->flags & LOS_PM_FLAGS_SYSCTRL) {
        sysctrl = (LosPmSysctrl *)opt;
        if (sysctrl->suspend == NULL) {
            return LOS_ERRNO_PM_INVALID_PARAM;
        }

        intSave = LOS_IntLock();
        pm->sysctrl = (LosPmSysctrl *)opt;
        LOS_IntRestore(intSave);
        return LOS_OK;
    }

    if ((opt->deepSuspend == NULL) && (opt->shutdownSuspend == NULL)) {
        return LOS_ERRNO_PM_INVALID_PARAM;
    }

    intSave = LOS_IntLock();
    LOS_ListTailInsert(&pm->head, &opt->list);
    LOS_IntRestore(intSave);
    return LOS_OK;
}

UINT32 LOS_PmUnregistered(LosPmDeviceOpt *opt)
{
    UINT32 intSave;
    LosPmCB *pm = &g_pmCB;
    LosPmDeviceOpt *listNode = NULL;
    LOS_DL_LIST *head = &g_pmCB.head;
    LOS_DL_LIST *list = head->pstNext;

    if (opt == NULL) {
        return LOS_ERRNO_PM_INVALID_PARAM;
    }

    intSave = LOS_IntLock();
#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
    if (pm->lowFreqTimer == (LosPmLowFreqTimer *)opt) {
        pm->lowFreqTimer = NULL;
        LOS_IntRestore(intSave);
        return LOS_OK;
    }
#endif

    if (pm->sysctrl == (LosPmSysctrl *)opt) {
        if (pm->sysctrl->flags & OS_PM_NODE_FREE) {
            (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, pm->sysctrl);
        }
        pm->sysctrl = NULL;
        LOS_IntRestore(intSave);
        return LOS_OK;
    }

    while (list != head) {
        listNode = LOS_DL_LIST_ENTRY(list, LosPmDeviceOpt, list);
        if (listNode == opt) {
            LOS_ListDelete(&listNode->list);
            LOS_IntRestore(intSave);
            return LOS_OK;
        }

        list = list->pstNext;
    }

    LOS_IntRestore(intSave);
    return LOS_ERRNO_PM_INVALID_NODE;
}

LOS_SysSleepEnum LOS_PmModeGet(VOID)
{
    LOS_SysSleepEnum mode;
    LosPmCB *pm = &g_pmCB;

    UINT32 intSave = LOS_IntLock();
    mode = pm->pmMode;
    LOS_IntRestore(intSave);

    return mode;
}

UINT32 LOS_PmModeSet(LOS_SysSleepEnum mode)
{
    UINT32 ret;
    LosPmCB *pm = &g_pmCB;
    UINT32 intSave;

    if ((mode < 0) || (mode > LOS_SYS_SHUTDOWN)) {
        return LOS_ERRNO_PM_INVALID_MODE;
    }

    intSave = LOS_IntLock();
    pm->pmMode = mode;
    LOS_IntRestore(intSave);

    return ret;
}

#if (LOSCFG_KERNEL_PM_DEBUG == 1)
VOID LOS_PmLockInfoShow(VOID)
{
    UINT32 intSave;
    LosPmCB *pm = &g_pmCB;
    OsPmLockCB *lock = NULL;
    LOS_DL_LIST *head = &pm->lockList;
    LOS_DL_LIST *list = head->pstNext;

    PRINTK("Name                          Count\n\r");

    intSave = LOS_IntLock();
    while (list != head) {
        lock = LOS_DL_LIST_ENTRY(list, OsPmLockCB, list);
        PRINTK("%-30s%5u\n\r", lock->name, lock->count);
        list = list->pstNext;
    }
    LOS_IntRestore(intSave);

    return;
}
#endif

UINT32 LOS_PmLockRequest(const CHAR *name)
{
    UINT32 intSave;
    UINT32 ret = LOS_ERRNO_PM_NOT_LOCK;
    LosPmCB *pm = &g_pmCB;
#if (LOSCFG_KERNEL_PM_DEBUG == 1)
    OsPmLockCB *lock = NULL;
    LOS_DL_LIST *head = &pm->lockList;
    LOS_DL_LIST *list = head->pstNext;

    if (name == NULL) {
        return LOS_ERRNO_PM_INVALID_PARAM;
    }
#endif

    intSave = LOS_IntLock();
#if (LOSCFG_KERNEL_PM_DEBUG == 1)
    while (list != head) {
        lock = LOS_DL_LIST_ENTRY(list, OsPmLockCB, list);
        if (strcmp(name, lock->name) == 0) {
            break;
        }

        list = list->pstNext;
    }

    if (lock == NULL) {
        lock = LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR, sizeof(OsPmLockCB));
        if (lock == NULL) {
            LOS_IntRestore(intSave);
            return LOS_NOK;
        }
        lock->name = (CHAR *)name;
        lock->count = 1;
        LOS_ListTailInsert(head, &lock->list);
    } else if (lock->count < OS_PM_LOCK_MAX) {
        lock->count++;
    }
#endif

    if (pm->lock < OS_PM_LOCK_MAX) {
        pm->lock++;
        ret = LOS_OK;
    }

    LOS_IntRestore(intSave);
    return ret;
}

UINT32 LOS_PmLockRelease(const CHAR *name)
{
    UINT32 intSave;
    UINT32 ret = LOS_ERRNO_PM_NOT_LOCK;
    LosPmCB *pm = &g_pmCB;
#if (LOSCFG_KERNEL_PM_DEBUG == 1)
    OsPmLockCB *lock = NULL;
    LOS_DL_LIST *head = &pm->lockList;
    LOS_DL_LIST *list = head->pstNext;
    VOID *lockFree = NULL;

    if (name == NULL) {
        return LOS_ERRNO_PM_INVALID_PARAM;
    }
#endif

    intSave = LOS_IntLock();
#if (LOSCFG_KERNEL_PM_DEBUG == 1)
    while (list != head) {
        lock = LOS_DL_LIST_ENTRY(list, OsPmLockCB, list);
        if (strcmp(name, lock->name) == 0) {
            break;
        }

        list = list->pstNext;
    }

    if (lock == NULL) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_PM_NOT_LOCK;
    } else if (lock->count > 0) {
        lock->count--;
        if (lock->count == 0) {
            LOS_ListDelete(&lock->list);
            lockFree = lock;
        }
    }
#endif

    if (pm->lock > 0) {
        pm->lock--;
        ret = LOS_OK;
    }

    LOS_IntRestore(intSave);
#if (LOSCFG_KERNEL_PM_DEBUG == 1)
    (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, lockFree);
#endif
    return ret;
}

UINT32 LOS_PmInit(VOID)
{
    UINT32 ret;
    LosPmSysctrl *sysctrl = NULL;
    LosPmCB *pm = &g_pmCB;

    (VOID)memset_s(pm, sizeof(LosPmCB), 0, sizeof(LosPmCB));

    pm->pmMode = LOS_SYS_NORMAL_SLEEP;
    LOS_ListInit(&pm->head);
#if (LOSCFG_KERNEL_PM_DEBUG == 1)
    LOS_ListInit(&pm->lockList);
#endif

    sysctrl = (LosPmSysctrl *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR, sizeof(LosPmSysctrl));
    if (sysctrl == NULL) {
        return LOS_NOK;
    }

    sysctrl->suspend = HalEnterSleep;
    sysctrl->resume = NULL;
    sysctrl->flags = OS_PM_NODE_FREE | LOS_PM_FLAGS_SYSCTRL;
    ret = LOS_PmRegistered((VOID *)sysctrl);
    if (ret != LOS_OK) {
        (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, sysctrl);
    }

    return ret;
}
#endif

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

typedef struct {
    LOS_SysSleepEnum  mode;
    UINT16            deepEnable;
    UINT16            deepLock;
    UINT16            shutdownEnable;
    UINT16            shutdownLock;
    LOS_DL_LIST       head;         /* devices list */
    LosPmSysctrl      *sysctrl;
#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
    LosPmLowFreqTimer *lowFreqTimer;
#endif
} LosPmCB;

STATIC LosPmCB g_pmCB;

STATIC LOS_SysSleepEnum OsPmModeGet(VOID)
{
    LosPmCB *pm = &g_pmCB;
    LOS_SysSleepEnum mode = LOS_SYS_NORMAL_SLEEP;

    if (pm->shutdownEnable && !pm->shutdownLock) {
        mode = LOS_SYS_SHUTDOWN;
    } else if (pm->deepEnable && !pm->deepLock) {
        mode = LOS_SYS_DEEP_SLEEP;
    }

    return mode;
}

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

    /* All devices are restored from low power mode */
    while (list != head) {
        opt = LOS_DL_LIST_ENTRY(list, LosPmDeviceOpt, list);
        if (opt->resume != NULL) {
            opt->resume(opt->device, pm->mode);
        }

        list = list->pstNext;
    }

    LOS_IntRestore(intSave);
    return;
}

VOID OsPmEnter(VOID)
{
    UINT32 intSave;
    PmSysctrlSuspend suspend = NULL;
    LosPmDeviceOpt *opt = NULL;
    LOS_DL_LIST *head = &g_pmCB.head;
    LOS_DL_LIST *list = head->pstNext;
    LosPmCB *pm = &g_pmCB;

    intSave = LOS_IntLock();
    pm->mode = OsPmModeGet();

    if (pm->mode != LOS_SYS_NORMAL_SLEEP) {
        /* All devices enter low power mode */
        while (list != head) {
            opt = LOS_DL_LIST_ENTRY(list, LosPmDeviceOpt, list);
            if (opt->suspend != NULL) {
                opt->suspend(opt->device, pm->mode);
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

    if (opt->suspend == NULL) {
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
    UINT32 intSave = LOS_IntLock();
    mode = OsPmModeGet();
    LOS_IntRestore(intSave);

    return mode;
}

UINT32 LOS_PmModeEnable(LOS_SysSleepEnum mode)
{
    UINT32 ret;
    LosPmCB *pm = &g_pmCB;

    switch (mode) {
        case LOS_SYS_NORMAL_SLEEP:
            return LOS_OK;
        case LOS_SYS_DEEP_SLEEP:
            if (!pm->deepEnable) {
                pm->deepEnable++;
                return LOS_OK;
            }
            ret = LOS_ERRNO_PM_MODE_ALREADY_ENABLE;
            break;
        case LOS_SYS_SHUTDOWN:
           if (!pm->shutdownEnable) {
               pm->shutdownEnable++;
               return LOS_OK;
           }
           ret = LOS_ERRNO_PM_MODE_ALREADY_ENABLE;
           break;
        default:
           ret = LOS_ERRNO_PM_INVALID_MODE;
            break;
    }

    return ret;
}

UINT32 LOS_PmModeDisable(LOS_SysSleepEnum mode)
{
    UINT32 ret;
    LosPmCB *pm = &g_pmCB;

    switch (mode) {
        case LOS_SYS_NORMAL_SLEEP:
            return LOS_OK;
        case LOS_SYS_DEEP_SLEEP:
            if (pm->deepEnable) {
                pm->deepEnable--;
                return LOS_OK;
            }

            ret = LOS_ERRNO_PM_MODE_NOT_ENABLE;
            break;
        case LOS_SYS_SHUTDOWN:
           if (pm->shutdownEnable) {
               pm->shutdownEnable--;
               return LOS_OK;
           }
           ret = LOS_ERRNO_PM_MODE_NOT_ENABLE;
           break;
        default:
           ret = LOS_ERRNO_PM_INVALID_MODE;
            break;
    }

    return ret;
}

UINT32 LOS_PmLockRequest(LOS_SysSleepEnum mode)
{
    UINT32 ret;
    LosPmCB *pm = &g_pmCB;

    switch (mode) {
        case LOS_SYS_NORMAL_SLEEP:
            return LOS_OK;
        case LOS_SYS_DEEP_SLEEP:
            if (pm->deepLock < OS_PM_LOCK_MAX) {
                pm->deepLock++;
                return LOS_OK;
            }
            ret = LOS_ERRNO_PM_LOCK_LIMIT;
            break;
        case LOS_SYS_SHUTDOWN:
           if (pm->shutdownLock < OS_PM_LOCK_MAX) {
               pm->shutdownLock++;
               return LOS_OK;
           }
           ret = LOS_ERRNO_PM_LOCK_LIMIT;
           break;
        default:
           ret = LOS_ERRNO_PM_INVALID_MODE;
            break;
    }

    return ret;
}

UINT32 LOS_PmLockRelease(LOS_SysSleepEnum mode)
{
    UINT32 ret;
    LosPmCB *pm = &g_pmCB;

    switch (mode) {
        case LOS_SYS_NORMAL_SLEEP:
            return LOS_OK;
        case LOS_SYS_DEEP_SLEEP:
            if (pm->deepLock > 0) {
                pm->deepLock--;
                return LOS_OK;
            }
            ret = LOS_ERRNO_PM_NOT_LOCK;
            break;
        case LOS_SYS_SHUTDOWN:
           if (pm->shutdownLock > 0) {
               pm->shutdownLock--;
               return LOS_OK;
           }
           ret = LOS_ERRNO_PM_NOT_LOCK;
           break;
        default:
           ret = LOS_ERRNO_PM_INVALID_MODE;
            break;
    }

    return ret;
}

UINT32 LOS_PmInit(VOID)
{
    UINT32 ret;
    LosPmSysctrl *sysctrl = NULL;

    (VOID)memset_s(&g_pmCB, sizeof(LosPmCB), 0, sizeof(LosPmCB));
    LOS_ListInit(&g_pmCB.head);

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

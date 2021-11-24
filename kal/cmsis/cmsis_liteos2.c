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

#include "cmsis_os2.h"
#include "kal.h"
#include "los_event.h"
#include "los_membox.h"
#include "los_memory.h"
#include "los_interrupt.h"
#include "los_mux.h"
#include "los_queue.h"
#include "los_sem.h"
#include "los_swtmr.h"
#include "los_task.h"
#include "los_timer.h"
#include "los_debug.h"

#include "string.h"
#include "securec.h"


/* Kernel initialization state */
static osKernelState_t g_kernelState;

extern BOOL g_taskScheduled;

/* LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO <---> osPriorityNormal */
#define LOS_PRIORITY(cmsisPriority) (LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO - ((cmsisPriority) - osPriorityNormal))
#define CMSIS_PRIORITY(losPriority) (osPriorityNormal + (LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO - (losPriority)))

/* OS_TASK_PRIORITY_HIGHEST and OS_TASK_PRIORITY_LOWEST is reserved for internal TIMER and IDLE task use only. */
#define ISVALID_LOS_PRIORITY(losPrio) ((losPrio) > OS_TASK_PRIORITY_HIGHEST && (losPrio) < OS_TASK_PRIORITY_LOWEST)

const osVersion_t g_stLosVersion = { 001, 001 };

#define LITEOS_VERSION_MAJOR 1
#define LITEOS_VERSION_MINOR 0
#define LITEOS_VERSION_BUILD 0

/* Kernel version and identification string definition */
#define KERNEL_VERSION            (((UINT32)LITEOS_VERSION_MAJOR * 10000000UL) | \
                                   ((UINT32)LITEOS_VERSION_MINOR *    10000UL) | \
                                   ((UINT32)LITEOS_VERSION_BUILD *        1UL))

#define KERNEL_ID "HUAWEI-LiteOS"

#define KERNEL_UNLOCKED 0
#define KERNEL_LOCKED   1

//  ==== Kernel Management Functions ====

osStatus_t osKernelInitialize(void)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    if (g_kernelState != osKernelInactive) {
        return osError;
    }

    if (LOS_OK == LOS_KernelInit()) {
        g_kernelState = osKernelReady;
        return osOK;
    } else {
        return osError;
    }
}


osStatus_t osKernelGetInfo(osVersion_t *version, char *id_buf, uint32_t id_size)
{
    errno_t ret;

    if ((version == NULL) || (id_buf == NULL) || (id_size == 0)) {
        return osError;
    }

    ret = memcpy_s(id_buf, id_size, KERNEL_ID, sizeof(KERNEL_ID));
    if (ret == EOK) {
        version->api = g_stLosVersion.api;
        version->kernel = g_stLosVersion.kernel;
        return osOK;
    } else {
        PRINT_ERR("[%s] memcpy_s failed, error type = %d\n", __func__, ret);
        return osError;
    }
}


osKernelState_t osKernelGetState(void)
{
    if (!g_taskScheduled) {
        if (g_kernelState == osKernelReady) {
            return osKernelReady;
        } else {
            return osKernelInactive;
        }
    } else if (g_losTaskLock > 0) {
        return osKernelLocked;
    } else {
        return osKernelRunning;
    }
}


osStatus_t osKernelStart(void)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (g_kernelState != osKernelReady) {
        return osError;
    }

    if (LOS_Start() == LOS_OK) {
        g_kernelState = osKernelRunning;
        return osOK;
    } else {
        return osError;
    }
}


int32_t osKernelLock(void)
{
    int32_t lock;

    if (OS_INT_ACTIVE) {
        return (int32_t)osErrorISR;
    }

    if (!g_taskScheduled) {
        return (int32_t)osError;
    }

    if (g_losTaskLock > 0) {
        lock = KERNEL_LOCKED;
    } else {
        LOS_TaskLock();
        lock = KERNEL_UNLOCKED;
    }

    return lock;
}


int32_t osKernelUnlock(void)
{
    int32_t lock;

    if (OS_INT_ACTIVE) {
        return (int32_t)osErrorISR;
    }

    if (!g_taskScheduled) {
        return (int32_t)osError;
    }

    if (g_losTaskLock > 0) {
        LOS_TaskUnlock();
        if (g_losTaskLock != 0) {
            return (int32_t)osError;
        }
        lock = KERNEL_LOCKED;
    } else {
        lock = KERNEL_UNLOCKED;
    }

    return lock;
}


int32_t osKernelRestoreLock(int32_t lock)
{
    if (OS_INT_ACTIVE) {
        return (int32_t)osErrorISR;
    }

    if (!g_taskScheduled) {
        return (int32_t)osError;
    }

    switch (lock) {
        case KERNEL_UNLOCKED:
            LOS_TaskUnlock();
            if (g_losTaskLock != 0) {
                break;
            }
            return KERNEL_UNLOCKED;
        case KERNEL_LOCKED:
            LOS_TaskLock();
            return KERNEL_LOCKED;
        default:
            break;
    }

    return (int32_t)osError;
}


uint32_t osKernelGetTickCount(void)
{
    uint64_t ticks = LOS_TickCountGet();
    return (uint32_t)ticks;
}

uint32_t osKernelGetTickFreq(void)
{
    return (uint32_t)LOSCFG_BASE_CORE_TICK_PER_SECOND;
}

uint32_t osKernelGetSysTimerCount(void)
{
    return (uint32_t)LOS_SysCycleGet;
}


uint32_t osKernelGetSysTimerFreq(void)
{
    return OS_SYS_CLOCK;
}


//  ==== Thread Management Functions ====

osThreadId_t osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr)
{
    UINT32 uwTid;
    UINT32 uwRet;
    LosTaskCB *pstTaskCB = NULL;
    TSK_INIT_PARAM_S stTskInitParam = {NULL};
    UINT16 usPriority;

    if (OS_INT_ACTIVE || (func == NULL)) {
        return (osThreadId_t)NULL;
    }

    usPriority = attr ? LOS_PRIORITY(attr->priority) : LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    if (!ISVALID_LOS_PRIORITY(usPriority)) {
        /* unsupported priority */
        return (osThreadId_t)NULL;
    }

    stTskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)func;
    stTskInitParam.uwArg = (UINT32)argument;
    stTskInitParam.uwStackSize = attr ? attr->stack_size : LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    stTskInitParam.pcName = (CHAR *)(attr ? attr->name : "[NULL]");
    stTskInitParam.usTaskPrio = usPriority;
    if (attr && attr->attr_bits == osThreadJoinable) {
        stTskInitParam.uwResved = LOS_TASK_ATTR_JOINABLE;
    }
    uwRet = LOS_TaskCreate(&uwTid, &stTskInitParam);

    if (LOS_OK != uwRet) {
        return (osThreadId_t)NULL;
    }

    pstTaskCB = OS_TCB_FROM_TID(uwTid);

    return (osThreadId_t)pstTaskCB;
}


const char *osThreadGetName(osThreadId_t thread_id)
{
    LosTaskCB *pstTaskCB = NULL;

    if (OS_INT_ACTIVE || thread_id == NULL) {
        return NULL;
    }

    pstTaskCB = (LosTaskCB *)thread_id;

    return pstTaskCB->taskName;
}


osThreadId_t osThreadGetId(void)
{
    return (osThreadId_t)(g_losTask.runTask);
}

void *osThreadGetArgument(void)
{
    if (OS_INT_ACTIVE) {
        return 0;
    }

    LosTaskCB *taskCb = (LosTaskCB *)osThreadGetId();
    if (taskCb == NULL) {
        return NULL;
    }
    return (void *)(taskCb->arg);
}

osThreadState_t osThreadGetState(osThreadId_t thread_id)
{
    UINT16 taskStatus;
    osThreadState_t stState;
    LosTaskCB *pstTaskCB = NULL;

    if (OS_INT_ACTIVE || thread_id == NULL) {
        return osThreadError;
    }

    pstTaskCB = (LosTaskCB *)thread_id;
    taskStatus = pstTaskCB->taskStatus;

    if (taskStatus & OS_TASK_STATUS_RUNNING) {
        stState = osThreadRunning;
    } else if (taskStatus & OS_TASK_STATUS_READY) {
        stState = osThreadReady;
    } else if (taskStatus &
        (OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND |
         OS_TASK_STATUS_SUSPEND | OS_TASK_STATUS_PEND_TIME)) {
        stState = osThreadBlocked;
    } else if (taskStatus & OS_TASK_STATUS_UNUSED) {
        stState = osThreadInactive;
    } else {
        stState = osThreadError;
    }

    return stState;
}


uint32_t osThreadGetStackSize(osThreadId_t thread_id)
{
    LosTaskCB *pstTaskCB = NULL;

    if (OS_INT_ACTIVE || thread_id == NULL) {
        return 0U;
    }

    pstTaskCB = (LosTaskCB *)thread_id;

    return pstTaskCB->stackSize;
}


uint32_t osTaskStackWaterMarkGet(UINT32 taskID)
{
    UINT32 uwCount = 0;
    UINT32 *ptopOfStack;
    UINT32 intSave;
    LosTaskCB *pstTaskCB = NULL;

    if (taskID > LOSCFG_BASE_CORE_TSK_LIMIT) {
        return 0;
    }

    intSave = LOS_IntLock();

    pstTaskCB = OS_TCB_FROM_TID(taskID);
    if (OS_TASK_STATUS_UNUSED & (pstTaskCB->taskStatus)) {
        LOS_IntRestore(intSave);
        return 0;
    }

    // first 4 bytes is OS_TASK_MAGIC_WORD, skip
    ptopOfStack = (UINT32 *)(UINTPTR)pstTaskCB->topOfStack + 1;

    while (*ptopOfStack == (UINT32)OS_TASK_STACK_INIT) {
        ++ptopOfStack;
        ++uwCount;
    }

    uwCount *= sizeof(UINT32);

    LOS_IntRestore(intSave);
    return uwCount;
}


uint32_t osThreadGetStackSpace(osThreadId_t thread_id)
{
    LosTaskCB *pstTaskCB = NULL;

    if (OS_INT_ACTIVE || thread_id == NULL) {
        return 0U;
    }

    pstTaskCB = (LosTaskCB *)thread_id;

    return osTaskStackWaterMarkGet(pstTaskCB->taskID);
}


osStatus_t osThreadSetPriority(osThreadId_t thread_id, osPriority_t priority)
{
    UINT32 uwRet;
    UINT16 usPriority;
    LosTaskCB *pstTaskCB = NULL;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    if (thread_id == NULL) {
        return osErrorParameter;
    }

    usPriority = LOS_PRIORITY(priority);
    if (!ISVALID_LOS_PRIORITY(usPriority)) {
        return osErrorParameter;
    }

    pstTaskCB = (LosTaskCB *)thread_id;
    uwRet = LOS_TaskPriSet(pstTaskCB->taskID, usPriority);
    switch (uwRet) {
        case LOS_ERRNO_TSK_PRIOR_ERROR:
        case LOS_ERRNO_TSK_OPERATE_IDLE:
        case LOS_ERRNO_TSK_ID_INVALID:
            return osErrorParameter;

        case LOS_ERRNO_TSK_NOT_CREATED:
            return osErrorResource;

        default:
            return osOK;
    }
}


osPriority_t osThreadGetPriority(osThreadId_t thread_id)
{
    UINT16 usRet;
    LosTaskCB *pstTaskCB = NULL;

    if (OS_INT_ACTIVE || thread_id == NULL) {
        return osPriorityError;
    }

    pstTaskCB = (LosTaskCB *)thread_id;
    usRet = LOS_TaskPriGet(pstTaskCB->taskID);

    if (usRet == (UINT16)OS_INVALID || usRet > OS_TASK_PRIORITY_LOWEST) {
        return osPriorityError;
    }

    return (osPriority_t)CMSIS_PRIORITY(usRet);
}


osStatus_t osThreadYield(void)
{
    UINT32 uwRet;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    uwRet = LOS_TaskYield();

    if (uwRet == LOS_OK) {
        return osOK;
    }

    return osError;
}


osStatus_t osThreadSuspend(osThreadId_t thread_id)
{
    UINT32 uwRet;
    LosTaskCB *pstTaskCB = NULL;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    if (thread_id == NULL) {
        return osErrorParameter;
    }

    pstTaskCB = (LosTaskCB *)thread_id;
    uwRet = LOS_TaskSuspend(pstTaskCB->taskID);
    switch (uwRet) {
        case LOS_ERRNO_TSK_OPERATE_IDLE:
        case LOS_ERRNO_TSK_SUSPEND_SWTMR_NOT_ALLOWED:
        case LOS_ERRNO_TSK_ID_INVALID:
            return osErrorParameter;

        case LOS_ERRNO_TSK_NOT_CREATED:
        case LOS_ERRNO_TSK_ALREADY_SUSPENDED:
        case LOS_ERRNO_TSK_SUSPEND_LOCKED:
            return osErrorResource;

        default:
            return osOK;
    }
}


osStatus_t osThreadResume(osThreadId_t thread_id)
{
    UINT32 uwRet;
    LosTaskCB *pstTaskCB = NULL;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    if (thread_id == NULL) {
        return osErrorParameter;
    }

    pstTaskCB = (LosTaskCB *)thread_id;
    uwRet = LOS_TaskResume(pstTaskCB->taskID);

    switch (uwRet) {
        case LOS_ERRNO_TSK_ID_INVALID:
            return osErrorParameter;

        case LOS_ERRNO_TSK_NOT_CREATED:
        case LOS_ERRNO_TSK_NOT_SUSPENDED:
            return osErrorResource;

        default:
            return osOK;
    }
}


osStatus_t osThreadDetach(osThreadId_t thread_id)
{
    UINT32 ret;
    LosTaskCB *taskCB = (LosTaskCB *)thread_id;

    if (thread_id == NULL) {
        return osErrorParameter;
    }

    ret = LOS_TaskDetach(taskCB->taskID);
    if (ret == LOS_ERRNO_TSK_NOT_ALLOW_IN_INT) {
        return osErrorISR;
    } else if (ret != LOS_OK) {
        return osErrorResource;
    }

    return osOK;
}


osStatus_t osThreadJoin(osThreadId_t thread_id)
{
    UINT32 ret;
    LosTaskCB *taskCB = (LosTaskCB *)thread_id;

    if (thread_id == NULL) {
        return osErrorParameter;
    }

    ret = LOS_TaskJoin(taskCB->taskID, NULL);
    if (ret == LOS_ERRNO_TSK_NOT_ALLOW_IN_INT) {
        return osErrorISR;
    } else if (ret != LOS_OK) {
        return osErrorResource;
    }

    return osOK;
}


osStatus_t osThreadTerminate(osThreadId_t thread_id)
{
    UINT32 uwRet;
    LosTaskCB *pstTaskCB = NULL;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    if (thread_id == NULL) {
        return osErrorParameter;
    }

    pstTaskCB = (LosTaskCB *)thread_id;
    uwRet = LOS_TaskDelete(pstTaskCB->taskID);

    switch (uwRet) {
        case LOS_ERRNO_TSK_OPERATE_IDLE:
        case LOS_ERRNO_TSK_SUSPEND_SWTMR_NOT_ALLOWED:
        case LOS_ERRNO_TSK_ID_INVALID:
            return osErrorParameter;

        case LOS_ERRNO_TSK_NOT_CREATED:
            return osErrorResource;

        default:
            return osOK;
    }
}


uint32_t osThreadGetCount(void)
{
    uint32_t uwCount = 0;

    if (OS_INT_ACTIVE) {
        return 0U;
    }

    for (uint32_t index = 0; index <= LOSCFG_BASE_CORE_TSK_LIMIT; index++) {
        if (!((g_taskCBArray + index)->taskStatus & OS_TASK_STATUS_UNUSED)) {
            uwCount++;
        }
    }

    return uwCount;
}


void osThreadExit(void)
{
    (void)LOS_TaskDelete(LOS_CurTaskIDGet());
    UNREACHABLE;
}


osStatus_t osDelay(uint32_t ticks)
{

    UINT32 uwRet = LOS_OK;
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (ticks == 0) {
        return osOK;
    }
    if (osKernelGetState() != osKernelRunning) {
        LOS_UDelay(ticks * OS_US_PER_TICK);
    } else {
        uwRet = LOS_TaskDelay(ticks);
    }
    if (uwRet == LOS_OK) {
        return osOK;
    } else {
        return osError;
    }
}


osStatus_t osDelayUntil(uint32_t ticks)
{
    UINT32 uwRet;
    UINT32 uwTicks;
    UINT32 tickCount = osKernelGetTickCount();

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    if (ticks < tickCount) {
        return osError;
    }

    uwTicks = (UINT32)(ticks - tickCount);

    uwRet = LOS_TaskDelay(uwTicks);
    if (uwRet == LOS_OK) {
        return osOK;
    } else {
        return osError;
    }
}

//  ==== Timer Management Functions ====
#if (LOSCFG_BASE_CORE_SWTMR == 1)
osTimerId_t osTimerNew(osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr)
{
    (VOID)attr;
    UINT32 swtmrId;
    UINT8 mode;

    if ((func == NULL) || OS_INT_ACTIVE) {
        return NULL;
    }

    if (type == osTimerOnce) {
        mode = LOS_SWTMR_MODE_NO_SELFDELETE;
    } else if (type == osTimerPeriodic) {
        mode = LOS_SWTMR_MODE_PERIOD;
    } else {
        return NULL;
    }

    if (osTimerOnce == type) {
        mode = LOS_SWTMR_MODE_NO_SELFDELETE;
    } else {
        mode = LOS_SWTMR_MODE_PERIOD;
    }
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    if (LOS_SwtmrCreate(1, mode, (SWTMR_PROC_FUNC)func, &swtmrId, (UINT32)(UINTPTR)argument,
        osTimerRousesAllow, osTimerAlignIgnore) != LOS_OK) {
        return (osTimerId_t)NULL;
    }
#else
    if (LOS_SwtmrCreate(1, mode, (SWTMR_PROC_FUNC)func, &swtmrId, (UINT32)(UINTPTR)argument) != LOS_OK) {
        return (osTimerId_t)NULL;
    }
#endif
    return (osTimerId_t)OS_SWT_FROM_SID(swtmrId);
}



osStatus_t osTimerStart(osTimerId_t timer_id, uint32_t ticks)
{
    UINT32 uwRet;
    SWTMR_CTRL_S *pstSwtmr;
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if ((ticks == 0) || (timer_id == NULL)) {
        return osErrorParameter;
    }

    UINT32 intSave = LOS_IntLock();
    pstSwtmr = (SWTMR_CTRL_S *)timer_id;
    pstSwtmr->uwInterval = ticks;
    uwRet = LOS_SwtmrStart(pstSwtmr->usTimerID);
    LOS_IntRestore(intSave);
    if (LOS_OK == uwRet) {
        return osOK;
    } else if (LOS_ERRNO_SWTMR_ID_INVALID == uwRet) {
        return osErrorParameter;
    } else {
        return osErrorResource;
    }
}


const char *osTimerGetName(osTimerId_t timer_id)
{
    UNUSED(timer_id);
    return (const char *)NULL;
}


osStatus_t osTimerStop(osTimerId_t timer_id)
{
    UINT32 uwRet;
    SWTMR_CTRL_S *pstSwtmr = (SWTMR_CTRL_S *)timer_id;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    if (pstSwtmr == NULL) {
        return osErrorParameter;
    }

    uwRet = LOS_SwtmrStop(pstSwtmr->usTimerID);
    if (LOS_OK == uwRet) {
        return osOK;
    } else if (LOS_ERRNO_SWTMR_ID_INVALID == uwRet) {
        return osErrorParameter;
    } else {
        return osErrorResource;
    }
}


uint32_t osTimerIsRunning(osTimerId_t timer_id)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (timer_id == NULL) {
        return 0;
    }

    return (OS_SWTMR_STATUS_TICKING == ((SWTMR_CTRL_S *)timer_id)->ucState);
}


osStatus_t osTimerDelete(osTimerId_t timer_id)
{
    UINT32 uwRet;
    SWTMR_CTRL_S *pstSwtmr = (SWTMR_CTRL_S *)timer_id;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (pstSwtmr == NULL) {
        return osErrorParameter;
    }
    uwRet = LOS_SwtmrDelete(pstSwtmr->usTimerID);
    if (uwRet == LOS_OK) {
        return osOK;
    } else if (LOS_ERRNO_SWTMR_ID_INVALID == uwRet) {
        return osErrorParameter;
    } else {
        return osErrorResource;
    }
}
#endif

osEventFlagsId_t osEventFlagsNew(const osEventFlagsAttr_t *attr)
{
    PEVENT_CB_S pstEventCB;
    UINT32 uwRet;

    UNUSED(attr);

    if (OS_INT_ACTIVE) {
        return (osEventFlagsId_t)NULL;
    }

    pstEventCB = (PEVENT_CB_S)LOS_MemAlloc(m_aucSysMem0, sizeof(EVENT_CB_S));
    if (pstEventCB == NULL) {
        return (osEventFlagsId_t)NULL;
    }

    uwRet = LOS_EventInit(pstEventCB);
    if (uwRet == LOS_OK) {
        return (osEventFlagsId_t)pstEventCB;
    } else {
        if (LOS_MemFree(m_aucSysMem0, pstEventCB) != LOS_OK) {
            PRINT_ERR("[%s] memory free fail!\n", __func__);
        }
        return NULL;
    }
}


const char *osEventFlagsGetName(osEventFlagsId_t ef_id)
{
    if (OS_INT_ACTIVE) {
        return NULL;
    }
    return NULL;
}


uint32_t osEventFlagsSet(osEventFlagsId_t ef_id, uint32_t flags)
{
    PEVENT_CB_S pstEventCB = (PEVENT_CB_S)ef_id;
    UINT32 uwRet;
    uint32_t rflags;

    uwRet = LOS_EventWrite(pstEventCB, (UINT32)flags);
    if (uwRet == LOS_OK) {
        rflags = pstEventCB->uwEventID;
        return rflags;
    } else {
        return (uint32_t)osFlagsErrorResource;
    }
}


uint32_t osEventFlagsClear(osEventFlagsId_t ef_id, uint32_t flags)
{
    PEVENT_CB_S pstEventCB = (PEVENT_CB_S)ef_id;
    UINT32 intSave;
    uint32_t rflags;
    UINT32 uwRet;

    if (pstEventCB == NULL) {
        return (uint32_t)osFlagsErrorParameter;
    }

    intSave = LOS_IntLock();
    rflags = pstEventCB->uwEventID;

    uwRet = LOS_EventClear(pstEventCB, ~flags);
    LOS_IntRestore(intSave);
    if (uwRet == LOS_OK) {
        return rflags;
    } else {
        return (uint32_t)osFlagsErrorResource;
    }
}


uint32_t osEventFlagsGet(osEventFlagsId_t ef_id)
{
    PEVENT_CB_S pstEventCB = (PEVENT_CB_S)ef_id;
    UINT32 intSave;
    uint32_t rflags;

    if (pstEventCB == NULL) {
        return 0;
    }

    intSave = LOS_IntLock();
    rflags = pstEventCB->uwEventID;
    LOS_IntRestore(intSave);

    return rflags;
}

uint32_t osEventFlagsWait(osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout)
{
    PEVENT_CB_S pstEventCB = (PEVENT_CB_S)ef_id;
    UINT32 uwMode = 0;
    UINT32 uwRet;
    uint32_t rflags;

    if (OS_INT_ACTIVE && (timeout != 0)) {
        return (uint32_t)osFlagsErrorParameter;
    }

    if (options > (osFlagsWaitAny | osFlagsWaitAll | osFlagsNoClear)) {
        return (uint32_t)osFlagsErrorParameter;
    }

    if ((options & osFlagsWaitAll) == osFlagsWaitAll) {
        uwMode |= LOS_WAITMODE_AND;
    } else {
        uwMode |= LOS_WAITMODE_OR;
    }

    if ((options & osFlagsNoClear) == osFlagsNoClear) {
        uwMode &= ~LOS_WAITMODE_CLR;
    } else {
        uwMode |= LOS_WAITMODE_CLR;
    }

    uwRet = LOS_EventRead(pstEventCB, (UINT32)flags, uwMode, (UINT32)timeout);
    switch (uwRet) {
        case LOS_ERRNO_EVENT_PTR_NULL:
        case LOS_ERRNO_EVENT_EVENTMASK_INVALID:
        case LOS_ERRNO_EVENT_FLAGS_INVALID:
        case LOS_ERRNO_EVENT_SETBIT_INVALID:
            return (uint32_t)osFlagsErrorParameter;

        case LOS_ERRNO_EVENT_READ_IN_INTERRUPT:
        case LOS_ERRNO_EVENT_READ_IN_LOCK:
            return (uint32_t)osFlagsErrorResource;

        case LOS_ERRNO_EVENT_READ_TIMEOUT:
            return (uint32_t)osFlagsErrorTimeout;

        default:
            rflags = (uint32_t)uwRet;
            return rflags;
    }
}

osStatus_t osEventFlagsDelete(osEventFlagsId_t ef_id)
{
    PEVENT_CB_S pstEventCB = (PEVENT_CB_S)ef_id;
    UINT32 intSave;
    osStatus_t uwRet;
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    intSave = LOS_IntLock();
    if (LOS_EventDestroy(pstEventCB) == LOS_OK) {
        uwRet = osOK;
    } else {
        uwRet = osErrorParameter;
    }
    LOS_IntRestore(intSave);

    if (LOS_MemFree(m_aucSysMem0, (void *)pstEventCB) == LOS_OK) {
        uwRet = osOK;
    } else {
        uwRet = osErrorParameter;
    }

    return uwRet;
}

//  ==== Mutex Management Functions ====
#if (LOSCFG_BASE_IPC_MUX == 1)
osMutexId_t osMutexNew(const osMutexAttr_t *attr)
{
    UINT32 uwRet;
    UINT32 uwMuxId;

    UNUSED(attr);

    if (OS_INT_ACTIVE) {
        return NULL;
    }

    uwRet = LOS_MuxCreate(&uwMuxId);
    if (uwRet == LOS_OK) {
        return (osMutexId_t)(GET_MUX(uwMuxId));
    } else {
        return (osMutexId_t)NULL;
    }
}


osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
    LosMuxCB *muxCB = (LosMuxCB *)mutex_id;
    UINT32 ret;

    if (muxCB == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    ret = LOS_MuxPend(muxCB->muxID, timeout);
    if (ret == LOS_OK) {
        return osOK;
    } else if (ret == LOS_ERRNO_MUX_INVALID) {
        return osErrorParameter;
    } else if (ret == LOS_ERRNO_MUX_TIMEOUT) {
        return osErrorTimeout;
    } else {
        return osErrorResource;
    }
}


osStatus_t osMutexRelease(osMutexId_t mutex_id)
{
    LosMuxCB *muxCB = (LosMuxCB *)mutex_id;
    UINT32 ret;

    if (muxCB == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    ret = LOS_MuxPost(muxCB->muxID);
    if (ret == LOS_OK) {
        return osOK;
    } else if (ret == LOS_ERRNO_MUX_INVALID) {
        return osErrorParameter;
    } else {
        return osErrorResource;
    }
}


osThreadId_t osMutexGetOwner(osMutexId_t mutex_id)
{
    UINT32 intSave;
    LosTaskCB *pstTaskCB;

    if (OS_INT_ACTIVE) {
        return NULL;
    }

    if (mutex_id == NULL) {
        return NULL;
    }

    intSave = LOS_IntLock();
    pstTaskCB = ((LosMuxCB *)mutex_id)->owner;
    LOS_IntRestore(intSave);

    return (osThreadId_t)pstTaskCB;
}


osStatus_t osMutexDelete(osMutexId_t mutex_id)
{
    UINT32 uwRet;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    if (mutex_id == NULL) {
        return osErrorParameter;
    }

    uwRet = LOS_MuxDelete(((LosMuxCB *)mutex_id)->muxID);
    if (uwRet == LOS_OK) {
        return osOK;
    } else if (uwRet == LOS_ERRNO_MUX_INVALID) {
        return osErrorParameter;
    } else {
        return osErrorResource;
    }
}
#endif

//  ==== Semaphore Management Functions ====
#if (LOSCFG_BASE_IPC_SEM == 1)

osSemaphoreId_t osSemaphoreNew(uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr)
{
    UINT32 uwRet;
    UINT32 uwSemId;

    UNUSED(attr);

    if ((initial_count > max_count) || (max_count > LOS_SEM_COUNT_MAX) || OS_INT_ACTIVE) {
        return NULL;
    }

    if (max_count == 1) {
        uwRet = LOS_BinarySemCreate((UINT16)initial_count, &uwSemId);
    } else {
        uwRet = LOS_SemCreate((UINT16)initial_count, &uwSemId);
    }

    if (uwRet == LOS_OK) {
        return (osSemaphoreId_t)(GET_SEM(uwSemId));
    } else {
        return (osSemaphoreId_t)NULL;
    }
}

const char *osSemaphoreGetName(osSemaphoreId_t semaphore_id)
{
    (VOID)semaphore_id;
    return NULL;
}

osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout)
{
    LosSemCB *semCB = (LosSemCB *)semaphore_id;
    UINT32 ret;

    if ((semCB == NULL) || (OS_INT_ACTIVE && (timeout != 0))) {
        return osErrorParameter;
    }

    ret = LOS_SemPend(semCB->semID, timeout);
    if (ret == LOS_OK) {
        return osOK;
    } else if (ret == LOS_ERRNO_SEM_INVALID) {
        return osErrorParameter;
    } else if (ret == LOS_ERRNO_SEM_TIMEOUT) {
        return osErrorTimeout;
    } else {
        return osErrorResource;
    }
}


osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id)
{
    UINT32 uwRet;

    if (semaphore_id == NULL) {
        return osErrorParameter;
    }

    uwRet = LOS_SemPost(((LosSemCB *)semaphore_id)->semID);
    if (uwRet == LOS_OK) {
        return osOK;
    } else if (uwRet == LOS_ERRNO_SEM_INVALID) {
        return osErrorParameter;
    } else {
        return osErrorResource;
    }
}

uint32_t osSemaphoreGetCount(osSemaphoreId_t semaphore_id)
{
    LosSemCB *semCB = (LosSemCB *)semaphore_id;
    UINT32 intSave;
    UINT16 count;

    if (semCB == NULL) {
        return 0;
    }

    intSave = LOS_IntLock();
    if (semCB->semStat == 0) {
        LOS_IntRestore(intSave);
        return 0;
    }

    count = semCB->semCount;
    LOS_IntRestore(intSave);

    return (uint32_t)count;
}


osStatus_t osSemaphoreDelete(osSemaphoreId_t semaphore_id)
{
    UINT32 uwRet;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    if (semaphore_id == NULL) {
        return osErrorParameter;
    }

    uwRet = LOS_SemDelete(((LosSemCB *)semaphore_id)->semID);
    if (uwRet == LOS_OK) {
        return osOK;
    } else if (uwRet == LOS_ERRNO_SEM_INVALID) {
        return osErrorParameter;
    } else {
        return osErrorResource;
    }
}
#endif


//  ==== Message Queue Management Functions ====
#if (LOSCFG_BASE_IPC_QUEUE == 1)
osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr)
{
    UINT32 uwQueueID;
    UINT32 uwRet;
    UNUSED(attr);
    osMessageQueueId_t handle;

    if (0 == msg_count || 0 == msg_size || OS_INT_ACTIVE) {
        return (osMessageQueueId_t)NULL;
    }

    uwRet = LOS_QueueCreate((char *)NULL, (UINT16)msg_count, &uwQueueID, 0, (UINT16)msg_size);
    if (uwRet == LOS_OK) {
        handle = (osMessageQueueId_t)(GET_QUEUE_HANDLE(uwQueueID));
    } else {
        handle = (osMessageQueueId_t)NULL;
    }

    return handle;
}

STATIC osStatus_t osMessageQueueOp(osMessageQueueId_t mq_id, VOID *msg_ptr, UINT32 timeout, QueueReadWrite rw)
{
    LosQueueCB *queueCB = (LosQueueCB *)mq_id;
    UINT32 ret;
    UINT32 bufferSize;

    if ((queueCB == NULL) || (msg_ptr == NULL) || (OS_INT_ACTIVE && (timeout != 0))) {
        return osErrorParameter;
    }

    bufferSize = (UINT32)(queueCB->queueSize - sizeof(UINT32));
    if (rw == OS_QUEUE_WRITE) {
        ret = LOS_QueueWriteCopy(queueCB->queueID, msg_ptr, bufferSize, timeout);
    } else {
        ret = LOS_QueueReadCopy(queueCB->queueID, msg_ptr, &bufferSize, timeout);
    }

    if (ret == LOS_OK) {
        return osOK;
    } else if ((ret == LOS_ERRNO_QUEUE_INVALID) || (ret == LOS_ERRNO_QUEUE_NOT_CREATE)) {
        return osErrorParameter;
    } else if (ret == LOS_ERRNO_QUEUE_TIMEOUT) {
        return osErrorTimeout;
    } else {
        return osErrorResource;
    }
}


osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
    (VOID)msg_prio;
    return osMessageQueueOp(mq_id, (VOID *)msg_ptr, (UINT32)timeout, OS_QUEUE_WRITE);
}


osStatus_t osMessageQueueGet(osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout)
{
    (VOID)msg_prio;
    return osMessageQueueOp(mq_id, (VOID *)msg_ptr, (UINT32)timeout, OS_QUEUE_READ);
}


uint32_t osMessageQueueGetCapacity(osMessageQueueId_t mq_id)
{
    uint32_t capacity;
    LosQueueCB *pstQueue = (LosQueueCB *)mq_id;

    if (pstQueue == NULL) {
        capacity = 0U;
    } else {
        capacity = pstQueue->queueLen;
    }

    return (capacity);
}

uint32_t osMessageQueueGetMsgSize(osMessageQueueId_t mq_id)
{
    uint32_t size;
    LosQueueCB *pstQueue = (LosQueueCB *)mq_id;

    if (pstQueue == NULL) {
        size = 0U;
    } else {
        size = pstQueue->queueSize - sizeof(UINT32);
    }

    return (size);
}


uint32_t osMessageQueueGetCount(osMessageQueueId_t mq_id)
{
    uint32_t count;
    LosQueueCB *pstQueue = (LosQueueCB *)mq_id;

    if (pstQueue == NULL) {
        count = 0U;
    } else {
        count = (uint32_t)(pstQueue->readWriteableCnt[OS_QUEUE_READ]);
    }
    return count;
}


uint32_t osMessageQueueGetSpace(osMessageQueueId_t mq_id)
{
    uint32_t space;
    LosQueueCB *pstQueue = (LosQueueCB *)mq_id;

    if (pstQueue == NULL) {
        space = 0U;
    } else {
        space = (uint32_t)pstQueue->readWriteableCnt[OS_QUEUE_WRITE];
    }
    return space;
}

osStatus_t osMessageQueueDelete(osMessageQueueId_t mq_id)
{
    LosQueueCB *pstQueue = (LosQueueCB *)mq_id;
    UINT32 uwRet;

    if (pstQueue == NULL) {
        return osErrorParameter;
    }

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    uwRet = LOS_QueueDelete((UINT32)pstQueue->queueID);
    if (uwRet == LOS_OK) {
        return osOK;
    } else if (uwRet == LOS_ERRNO_QUEUE_NOT_FOUND || uwRet == LOS_ERRNO_QUEUE_NOT_CREATE) {
        return osErrorParameter;
    } else {
        return osErrorResource;
    }
}

const char *osMessageQueueGetName(osMessageQueueId_t mq_id)
{
    (VOID)mq_id;
    return NULL;
}
#endif


#define MP_ALLOC        1U
#define MD_ALLOC        2U
#define MEM_POOL_VALID  0xFFEEFF00

typedef struct {
    LOS_MEMBOX_INFO poolInfo;
    void            *poolBase;
    uint32_t        poolSize;
    uint32_t        status;
    const char      *name;
} MemPoolCB;

osMemoryPoolId_t osMemoryPoolNew(uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr)
{
    MemPoolCB *mp = NULL;
    const char *name = NULL;
    LOS_MEMBOX_NODE *node = NULL;
    uint32_t memCB = 0;
    uint32_t memMP = 0;
    uint32_t size;
    uint32_t index;

    if (OS_INT_ACTIVE) {
        return NULL;
    }

    if ((block_count == 0) || (block_size == 0)) {
        return NULL;
    }

    size = block_count * block_size;

    if (attr != NULL) {
        if ((attr->cb_mem != NULL) && (attr->cb_size >= sizeof(MemPoolCB))) {
            memCB = 1;
        }

        if ((attr->mp_mem != NULL) &&
            (((UINTPTR)attr->mp_mem & 0x3) == 0) &&  /* 0x3: Check if array is 4-byte aligned. */
            (attr->mp_size >= size)) {
            memMP = 1;
        }
        name = attr->name;
    }

    if (memCB == 0) {
        mp = LOS_MemAlloc(OS_SYS_MEM_ADDR, sizeof(MemPoolCB));
        if (mp == NULL) {
            return NULL;
        }
        mp->status = MP_ALLOC;
    } else {
        mp = attr->cb_mem;
        mp->status = 0;
    }

    if (memMP == 0) {
        mp->poolBase = LOS_MemAlloc(OS_SYS_MEM_ADDR, size);
        if (mp->poolBase == NULL) {
            (void)LOS_MemFree(OS_SYS_MEM_ADDR, mp);
            return NULL;
        }
        mp->status |= MD_ALLOC;
    } else {
        mp->poolBase = attr->mp_mem;
    }
    mp->poolSize = size;
    mp->name = name;
    mp->poolInfo.uwBlkCnt = 0;
    mp->poolInfo.uwBlkNum = block_count;
    mp->poolInfo.uwBlkSize = block_size;

    node = (LOS_MEMBOX_NODE *)mp->poolBase;
    mp->poolInfo.stFreeList.pstNext = node;
    for (index = 0; index < block_count - 1; ++index) {
        node->pstNext = OS_MEMBOX_NEXT(node, block_size);
        node = node->pstNext;
    }
    node->pstNext = NULL;

    mp->status |= MEM_POOL_VALID;

    return mp;
}

void *osMemoryPoolAlloc(osMemoryPoolId_t mp_id, uint32_t timeout)
{
    MemPoolCB *mp = (MemPoolCB *)mp_id;
    LOS_MEMBOX_NODE *node = NULL;
    UINT32 intSave;

    UNUSED(timeout);

    if (mp_id == NULL) {
        return NULL;
    }

    intSave = LOS_IntLock();
    if ((mp->status & MEM_POOL_VALID) == MEM_POOL_VALID) {
        node = mp->poolInfo.stFreeList.pstNext;
        if (node != NULL) {
            mp->poolInfo.stFreeList.pstNext = node->pstNext;
            mp->poolInfo.uwBlkCnt++;
        }
    }
    LOS_IntRestore(intSave);

    return node;
}

osStatus_t osMemoryPoolFree(osMemoryPoolId_t mp_id, void *block)
{

    MemPoolCB *mp = (MemPoolCB *)mp_id;
    LOS_MEMBOX_NODE *node = NULL;
    LOS_MEMBOX_NODE *nodeTmp = NULL;
    UINT32 intSave;

    if ((mp_id == NULL) || (block == NULL)) {
        return osErrorParameter;
    }

    intSave = LOS_IntLock();
    if ((mp->status & MEM_POOL_VALID) != MEM_POOL_VALID) {
        LOS_IntRestore(intSave);
        return osErrorResource;
    }

    if (((UINTPTR)block < (UINTPTR)mp->poolBase) ||
        ((UINTPTR)block >= ((UINTPTR)mp->poolBase + (UINTPTR)mp->poolSize))) {
        LOS_IntRestore(intSave);
        return osErrorParameter;
    }

    node = (LOS_MEMBOX_NODE *)block;
    nodeTmp = mp->poolInfo.stFreeList.pstNext;
    mp->poolInfo.stFreeList.pstNext = node;
    node->pstNext = nodeTmp;
    mp->poolInfo.uwBlkCnt--;
    LOS_IntRestore(intSave);

    return osOK;
}

osStatus_t osMemoryPoolDelete(osMemoryPoolId_t mp_id)
{
    MemPoolCB *mp = (MemPoolCB *)mp_id;
    UINT32 intSave;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }

    if (mp_id == NULL) {
        return osErrorParameter;
    }

    intSave = LOS_IntLock();
    if ((mp->status & MEM_POOL_VALID) != MEM_POOL_VALID) {
        LOS_IntRestore(intSave);
        return osErrorResource;
    }

    if (mp->status & MD_ALLOC) {
        (void)LOS_MemFree(OS_SYS_MEM_ADDR, mp->poolBase);
        mp->poolBase = NULL;
    }

    mp->name = NULL;
    mp->status &= ~MEM_POOL_VALID;

    if (mp->status & MP_ALLOC) {
        (void)LOS_MemFree(OS_SYS_MEM_ADDR, mp);
    }
    LOS_IntRestore(intSave);

    return osOK;
}

uint32_t osMemoryPoolGetCapacity(osMemoryPoolId_t mp_id)
{
    MemPoolCB *mp = (MemPoolCB *)mp_id;
    UINT32 intSave;
    uint32_t num;

    if (mp_id == NULL) {
        return 0;
    }

    intSave = LOS_IntLock();
    if ((mp->status & MEM_POOL_VALID) != MEM_POOL_VALID) {
        num = 0;
    } else {
        num = mp->poolInfo.uwBlkNum;
    }
    LOS_IntRestore(intSave);

    return num;
}

uint32_t osMemoryPoolGetBlockSize(osMemoryPoolId_t mp_id)
{
    MemPoolCB *mp = (MemPoolCB *)mp_id;
    UINT32 intSave;
    uint32_t size;

    if (mp_id == NULL) {
        return 0;
    }

    intSave = LOS_IntLock();
    if ((mp->status & MEM_POOL_VALID) != MEM_POOL_VALID) {
        size = 0;
    } else {
        size = mp->poolInfo.uwBlkSize;
    }
    LOS_IntRestore(intSave);

    return size;
}

uint32_t osMemoryPoolGetCount(osMemoryPoolId_t mp_id)
{
    MemPoolCB *mp = (MemPoolCB *)mp_id;
    UINT32 intSave;
    uint32_t count;

    if (mp_id == NULL) {
        return 0;
    }

    intSave = LOS_IntLock();
    if ((mp->status & MEM_POOL_VALID) != MEM_POOL_VALID) {
        count = 0;
    } else {
        count = mp->poolInfo.uwBlkCnt;
    }
    LOS_IntRestore(intSave);

    return count;
}

uint32_t osMemoryPoolGetSpace(osMemoryPoolId_t mp_id)
{
    MemPoolCB *mp = (MemPoolCB *)mp_id;
    UINT32 intSave;
    uint32_t space;

    if (mp_id == NULL) {
        return 0;
    }

    intSave = LOS_IntLock();
    if ((mp->status & MEM_POOL_VALID) != MEM_POOL_VALID) {
        space = 0;
    } else {
        space = mp->poolInfo.uwBlkCnt - mp->poolInfo.uwBlkCnt;
    }
    LOS_IntRestore(intSave);

    return space;

}

const char *osMemoryPoolGetName(osMemoryPoolId_t mp_id)
{
    MemPoolCB *mp = (MemPoolCB *)mp_id;
    const char *p = NULL;
    UINT32 intSave;

    if (mp_id == NULL) {
        return NULL;
    }

    if (OS_INT_ACTIVE) {
        return NULL;
    }

    intSave = LOS_IntLock();
    if ((mp->status & MEM_POOL_VALID) == MEM_POOL_VALID) {
        p = mp->name;
    }
    LOS_IntRestore(intSave);

    return p;
}

//  ==== Thread Flags Functions ====
uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    LosTaskCB *taskCB = (LosTaskCB *)thread_id;
    UINT32 ret;
    EVENT_CB_S *eventCB = NULL;
    UINT32 eventSave;

    if (taskCB == NULL) {
        return (uint32_t)osFlagsErrorParameter;
    }

    eventCB = &(taskCB->event);
    eventSave = eventCB->uwEventID;
    ret = LOS_EventWrite(eventCB, (UINT32)flags);
    if (ret == LOS_OK) {
        return ((uint32_t)eventSave | flags);
    } else if (ret == LOS_ERRNO_EVENT_SETBIT_INVALID) {
        return (uint32_t)osFlagsErrorParameter;
    } else {
        return (uint32_t)osFlagsErrorResource;
    }
}

uint32_t osThreadFlagsClear(uint32_t flags)
{
    UINT32 ret;
    UINT32 saveFlags;
    LosTaskCB *runTask = NULL;
    EVENT_CB_S *eventCB = NULL;

    if (OS_INT_ACTIVE) {
        return (uint32_t)osFlagsErrorUnknown;
    }

    runTask = g_losTask.runTask;
    eventCB = &(runTask->event);
    saveFlags = eventCB->uwEventID;

    ret = LOS_EventClear(eventCB, ~(UINT32)flags);
    if (ret == LOS_OK) {
        return (uint32_t)saveFlags;
    }

    return (uint32_t)osFlagsErrorResource;
}

uint32_t osThreadFlagsGet(void)
{
    LosTaskCB *runTask = NULL;
    EVENT_CB_S *eventCB = NULL;

    if (OS_INT_ACTIVE) {
        return (uint32_t)osFlagsErrorUnknown;
    }

    runTask = g_losTask.runTask;
    eventCB = &(runTask->event);

    return (uint32_t)(eventCB->uwEventID);
}

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{
    UINT32 ret;
    UINT32 mode = 0;
    LosTaskCB *runTask = NULL;
    EVENT_CB_S *eventCB = NULL;

    if (OS_INT_ACTIVE) {
        return (uint32_t)osFlagsErrorUnknown;
    }

    if (options > (osFlagsWaitAny | osFlagsWaitAll | osFlagsNoClear)) {
        return (uint32_t)osFlagsErrorParameter;
    }

    if ((options & osFlagsWaitAll) == osFlagsWaitAll) {
        mode |= LOS_WAITMODE_AND;
    } else {
        mode |= LOS_WAITMODE_OR;
    }

    if ((options & osFlagsNoClear) == osFlagsNoClear) {
        mode &= ~LOS_WAITMODE_CLR;
    } else {
        mode |= LOS_WAITMODE_CLR;
    }

    runTask = g_losTask.runTask;
    eventCB = &(runTask->event);

    ret = LOS_EventRead(eventCB, (UINT32)flags, mode, (UINT32)timeout);
    if (!(ret & LOS_ERRTYPE_ERROR)) {
        return (uint32_t)eventCB->uwEventID | ret;
    }

    switch (ret) {
        case LOS_ERRNO_EVENT_PTR_NULL:
        case LOS_ERRNO_EVENT_SETBIT_INVALID:
        case LOS_ERRNO_EVENT_EVENTMASK_INVALID:
        case LOS_ERRNO_EVENT_FLAGS_INVALID:
            return (uint32_t)osFlagsErrorParameter;
        case LOS_ERRNO_EVENT_READ_TIMEOUT:
            return (uint32_t)osFlagsErrorTimeout;
        default:
            return (uint32_t)osFlagsErrorResource;
    }
}

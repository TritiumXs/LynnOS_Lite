/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_task.h"
#include "securec.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_interrupt.h"
#include "los_memory.h"
#include "los_mpu.h"
#include "los_sched.h"
#include "los_mux.h"
#include "los_sem.h"
#include "los_timer.h"
#if (LOSCFG_BASE_CORE_CPUP == 1)
#include "los_cpup.h"
#endif
#if (LOSCFG_KERNEL_PM == 1)
#include "los_pm.h"
#endif

/**
 * @ingroup los_task
 * @brief Convenience macro for bitwise operation of task module
 */
#define EVALUATE_L(NUMBER, VALUE)  \
            ((NUMBER) = (((NUMBER) & OS_TSK_HIGH_BITS_MASK) | (VALUE)))

#define EVALUATE_H(NUMBER, VALUE)  \
            ((NUMBER) = (((NUMBER) & OS_TSK_LOW_BITS_MASK) | ((VALUE) << OS_TSK_LOW_BITS)))

#define UWROLLNUMSUB(NUMBER1, NUMBER2)  \
            ((NUMBER1) = (((NUMBER1) & OS_TSK_HIGH_BITS_MASK) | (UWROLLNUM(NUMBER1) - UWROLLNUM(NUMBER2))))

#define UWROLLNUMADD(NUMBER1, NUMBER2)  \
            ((NUMBER1) = (((NUMBER1) & OS_TSK_HIGH_BITS_MASK) | (UWROLLNUM(NUMBER1) + UWROLLNUM(NUMBER2))))

#define UWROLLNUM(NUMBER) ((NUMBER) & OS_TSK_LOW_BITS_MASK)

#define UWSORTINDEX(NUMBER) ((NUMBER) >> OS_TSK_LOW_BITS)

#define UWROLLNUMDEC(NUMBER)  \
            ((NUMBER) = ((NUMBER) - 1))

/**
 * @ingroup los_task
 * @brief check task id's validation
 */
#define OS_TASK_ID_CHECK(taskID)              (OS_TSK_GET_INDEX(taskID) < g_taskMaxNum)

/**
 * @ingroup los_task
 * @brief check task id's invalidation
 */
#define OS_CHECK_TSK_PID_NOIDLE(taskID)       (OS_TSK_GET_INDEX(taskID) >= g_taskMaxNum)

/**
 * @ingroup los_task
 * @brief the offset of task stack's top for skipping the magic word
 */
#define OS_TASK_STACK_TOP_OFFSET                4

#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 1)
/**
 * @ingroup los_task
 * @brief the size of task stack's protection area
 */
#define OS_TASK_STACK_PROTECT_SIZE              32
#endif

LITE_OS_SEC_BSS  LosTaskCB                           *g_taskCBArray = NULL;
LITE_OS_SEC_BSS  LosTask                             g_losTask[LOSCFG_KERNEL_CORE_NUM] = {0};
LITE_OS_SEC_BSS  UINT16                              g_losTaskLock[LOSCFG_KERNEL_CORE_NUM] = {0};
LITE_OS_SEC_BSS  UINT32                              g_taskMaxNum;
LITE_OS_SEC_BSS  UINT32                              g_idleTaskID[LOSCFG_KERNEL_CORE_NUM];

#if (LOSCFG_BASE_CORE_SWTMR == 1)
LITE_OS_SEC_BSS  UINT32                              g_swtmrTaskID;
#endif

LITE_OS_SEC_DATA_INIT LOS_DL_LIST                    g_losFreeTask;
LITE_OS_SEC_DATA_INIT LOS_DL_LIST                    g_taskRecycleList;
LITE_OS_SEC_BSS  BOOL                                g_taskScheduled;
LITE_OS_SEC_BSS  SPIN_LOCK_INIT(g_taskSpin);

STATIC VOID (*PmEnter)(VOID) = NULL;

#if (LOSCFG_BASE_CORE_EXC_TSK_SWITCH == 1)
TaskSwitchInfo g_taskSwitchInfo;
#endif

STATIC_INLINE UINT32 OsCheckTaskIDValid(UINT32 taskID)
{
    UINT32 ret = LOS_OK;
    if (OsIsIdleTask(taskID)) {
        ret = LOS_ERRNO_TSK_OPERATE_IDLE;
#if (LOSCFG_BASE_CORE_SWTMR == 1)
    } else if (taskID == g_swtmrTaskID) {
        ret = LOS_ERRNO_TSK_SUSPEND_SWTMR_NOT_ALLOWED;
#endif
    } else if (OS_TSK_GET_INDEX(taskID) >= g_taskMaxNum) {
        ret = LOS_ERRNO_TSK_ID_INVALID;
    }
    return ret;
}

STATIC INLINE VOID OsInsertTCBToFreeList(LosTaskCB *taskCB)
{
    UINT32 taskID = taskCB->taskID;
    (VOID)memset_s(taskCB, sizeof(LosTaskCB), 0, sizeof(LosTaskCB));
    taskCB->taskID = taskID;
    taskCB->taskStatus = OS_TASK_STATUS_UNUSED;
    LOS_ListAdd(&g_losFreeTask, &taskCB->pendList);
}

STATIC VOID OsRecycleTaskResources(LosTaskCB *taskCB, UINTPTR *stackPtr)
{
    if ((taskCB->taskStatus & OS_TASK_FLAG_STACK_FREE) && (taskCB->topOfStack != 0)) {
#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 1)
        *stackPtr = taskCB->topOfStack - OS_TASK_STACK_PROTECT_SIZE;
#else
        *stackPtr = taskCB->topOfStack;
#endif
        taskCB->topOfStack = (UINT32)NULL;
        taskCB->taskStatus &= ~OS_TASK_FLAG_STACK_FREE;
    }
    if (!(taskCB->taskStatus & OS_TASK_FLAG_JOINABLE)) {
        OsInsertTCBToFreeList(taskCB);
    }
}

STATIC VOID OsRecycleFinishedTask(VOID)
{
    LosTaskCB *taskCB = NULL;
    UINT32 intSave;
    UINTPTR stackPtr;

    SCHEDULER_LOCK(intSave);
    while (!LOS_ListEmpty(&g_taskRecycleList)) {
        taskCB = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&g_taskRecycleList));
        LOS_ListDelete(LOS_DL_LIST_FIRST(&g_taskRecycleList));
        stackPtr = 0;
        OsRecycleTaskResources(taskCB, &stackPtr);
        SCHEDULER_UNLOCK(intSave);

        (VOID)LOS_MemFree(OS_TASK_STACK_ADDR, (VOID *)stackPtr);
        SCHEDULER_LOCK(intSave);
    }
    SCHEDULER_UNLOCK(intSave);
}

UINT32 OsPmEnterHandlerSet(VOID (*func)(VOID))
{
    if (func == NULL) {
        return LOS_NOK;
    }

    PmEnter = func;
    return LOS_OK;
}

/*****************************************************************************
 Function    : OsIdleTask
 Description : Idle task.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT VOID OsIdleTask(VOID)
{
    while (1) {
        OsRecycleFinishedTask();

        if (PmEnter != NULL) {
            PmEnter();
        } else {
            (VOID)ArchEnterSleep();
        }
    }
}

/*****************************************************************************
 Function    : OsConvertTskStatus
 Description : Convert task status to string.
 Input       : taskStatus    --- task status
 Output      : None
 Return      : string
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT8 *OsConvertTskStatus(UINT16 taskStatus)
{
    if (taskStatus & OS_TASK_STATUS_RUNNING) {
        return (UINT8 *)"Running";
    } else if (taskStatus & OS_TASK_STATUS_READY) {
        return (UINT8 *)"Ready";
    } else if (taskStatus & OS_TASK_STATUS_EXIT) {
        return (UINT8 *)"Exit";
    } else if (taskStatus & OS_TASK_STATUS_SUSPEND) {
        return (UINT8 *)"Suspend";
    } else if (taskStatus & OS_TASK_STATUS_DELAY) {
        return (UINT8 *)"Delay";
    } else if (taskStatus & OS_TASK_STATUS_PEND) {
        if (taskStatus & OS_TASK_STATUS_PEND_TIME) {
            return (UINT8 *)"PendTime";
        }
        return (UINT8 *)"Pend";
    }

    return (UINT8 *)"Impossible";
}

UINT32 OsGetTaskWaterLine(UINT32 taskID)
{
    UINT32 *stackPtr = NULL;
    UINT32 peakUsed;

    if (*(UINT32 *)(UINTPTR)OS_TCB_FROM_TID(taskID)->topOfStack == OS_TASK_MAGIC_WORD) {
        stackPtr = (UINT32 *)(UINTPTR)(OS_TCB_FROM_TID(taskID)->topOfStack + OS_TASK_STACK_TOP_OFFSET);
        while ((stackPtr < (UINT32 *)(OS_TCB_FROM_TID(taskID)->stackPointer)) && (*stackPtr == OS_TASK_STACK_INIT)) {
            stackPtr += 1;
        }
        peakUsed = OS_TCB_FROM_TID(taskID)->stackSize -
            ((UINT32)(UINTPTR)stackPtr - OS_TCB_FROM_TID(taskID)->topOfStack);
    } else {
        PRINT_ERR("CURRENT task %s stack overflow!\n", OS_TCB_FROM_TID(taskID)->taskName);
        peakUsed = OS_NULL_INT;
    }
    return peakUsed;
}

#if (LOSCFG_BASE_CORE_CPUP == 1)
STATIC UINT32 GetAllTskCpupInfo(CPUP_INFO_S **cpuLessOneSec,
                                CPUP_INFO_S **cpuTenSec,
                                CPUP_INFO_S **cpuOneSec)
{
    if ((cpuLessOneSec == NULL) || (cpuTenSec == NULL) || (cpuOneSec == NULL)) {
        return OS_ERROR;
    }
    *cpuLessOneSec = (CPUP_INFO_S *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR, sizeof(CPUP_INFO_S) * g_taskMaxNum);
    if (*cpuLessOneSec == NULL) {
        PRINT_ERR("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
        return OS_ERROR;
    }
    // Ignore the return code when matching CSEC rule 6.6(3).
    (VOID)memset_s((VOID *)(*cpuLessOneSec), sizeof(CPUP_INFO_S) * g_taskMaxNum,
                   (INT32)0, sizeof(CPUP_INFO_S) * g_taskMaxNum);

    *cpuTenSec = (CPUP_INFO_S *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR, sizeof(CPUP_INFO_S) * g_taskMaxNum);
    if (*cpuTenSec == NULL) {
        PRINT_ERR("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
        (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, *cpuLessOneSec);
        *cpuLessOneSec = NULL;
        return OS_ERROR;
    }
    // Ignore the return code when matching CSEC rule 6.6(3).
    (VOID)memset_s((VOID *)(*cpuTenSec), sizeof(CPUP_INFO_S) * g_taskMaxNum,
                   (INT32)0, sizeof(CPUP_INFO_S) * g_taskMaxNum);

    *cpuOneSec = (CPUP_INFO_S *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR, sizeof(CPUP_INFO_S) * g_taskMaxNum);
    if (*cpuOneSec == NULL) {
        PRINT_ERR("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
        (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, *cpuLessOneSec);
        (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, *cpuTenSec);
        return OS_ERROR;
    }
    // Ignore the return code when matching CSEC rule 6.6(3).
    (VOID)memset_s((VOID *)(*cpuOneSec), sizeof(CPUP_INFO_S) * g_taskMaxNum,
                   (INT32)0, sizeof(CPUP_INFO_S) * g_taskMaxNum);

    LOS_TaskLock();
    (VOID)LOS_AllTaskCpuUsage(*cpuLessOneSec, CPUP_LESS_THAN_1S);
    (VOID)LOS_AllTaskCpuUsage(*cpuTenSec, CPUP_IN_10S);
    (VOID)LOS_AllTaskCpuUsage(*cpuOneSec, CPUP_IN_1S);
    LOS_TaskUnlock();

    return LOS_OK;
}
#endif

STATIC VOID PrintTskInfo(const LosTaskCB *taskCB)
{
    UINT32 semID;

    if (taskCB->taskStatus & OS_TASK_STATUS_EXIT) {
        PRINTK("%4u%9u%10s%#10x%#10x%#11x%#11x%#10x%#7x",
               taskCB->taskID, taskCB->priority, OsConvertTskStatus(taskCB->taskStatus),
               taskCB->stackSize, 0, 0, 0, 0, 0);
        return;
    }

    semID = (taskCB->taskSem == NULL) ? OS_NULL_SHORT : (((LosSemCB *)taskCB->taskSem)->semID);
    PRINTK("%4u%9u%10s%#10x%#10x%#11x%#11x%#10x%#7x",
           taskCB->taskID, taskCB->priority, OsConvertTskStatus(taskCB->taskStatus),
           taskCB->stackSize, OsGetTaskWaterLine(taskCB->taskID),
           (UINT32)(UINTPTR)taskCB->stackPointer, taskCB->topOfStack, taskCB->eventMask, semID);
    return;
}

STATIC VOID PrintTskInfoHeader(VOID)
{
    PRINTK("\r\n TID  Priority   Status StackSize WaterLine StackPoint TopOfStack EventMask  SemID");
#if (LOSCFG_TASK_MEM_USED == 1)
    PRINTK(" AllocSize");
#endif
#if (LOSCFG_BASE_CORE_CPUP == 1)
    PRINTK("  CPUUSE CPUUSE10s CPUUSE1s ");
#endif /* LOSCFG_BASE_CORE_CPUP */
#if defined(LOSCFG_KERNEL_SMP)
    PRINTK(" Affi Cpu ");
#endif /* LOSCFG_KERNEL_SMP */
    PRINTK("  TaskEntry name\n");
    PRINTK(" ---  -------- -------- ");
    PRINTK("--------- --------- ---------- ---------- --------- ------ ");
#if (LOSCFG_TASK_MEM_USED == 1)
    PRINTK("--------- ");
#endif
#if (LOSCFG_BASE_CORE_CPUP == 1)
    PRINTK("------- --------- --------  ");
#endif /* LOSCFG_BASE_CORE_CPUP */
#if defined(LOSCFG_KERNEL_SMP)
    PRINTK("---- ");
#endif /* LOSCFG_KERNEL_SMP */
    PRINTK("---------- ----\n");
}

#if (LOSCFG_TASK_MEM_USED == 1)
STATIC UINT32                              g_taskMemUsed[LOSCFG_BASE_CORE_TSK_LIMIT + 1];
#endif

/*****************************************************************************
 Function    : OsGetAllTskInfo
 Description : Get all task info.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 OsGetAllTskInfo(VOID)
{
#if (LOSCFG_KERNEL_PRINTF != 0)
    LosTaskCB    *taskCB = (LosTaskCB *)NULL;
    UINT32       loopNum;
#if (LOSCFG_BASE_CORE_CPUP == 1)
    CPUP_INFO_S *cpuLessOneSec = (CPUP_INFO_S *)NULL;
    CPUP_INFO_S *cpuTenSec = (CPUP_INFO_S *)NULL;
    CPUP_INFO_S *cpuOneSec = (CPUP_INFO_S *)NULL;
#endif

#if (LOSCFG_TASK_MEM_USED == 1)
    (VOID)memset_s(g_taskMemUsed, sizeof(UINT32) * g_taskMaxNum, 0, sizeof(UINT32) * g_taskMaxNum);
    OsTaskMemUsed((VOID *)OS_SYS_MEM_ADDR, g_taskMemUsed, g_taskMaxNum);
#endif

#if (LOSCFG_BASE_CORE_CPUP == 1)
    if (GetAllTskCpupInfo(&cpuLessOneSec, &cpuTenSec, &cpuOneSec) != LOS_OK) {
        return OS_ERROR;
    }
#endif /* LOSCFG_BASE_CORE_CPUP */

    PrintTskInfoHeader();

    for (loopNum = 0; loopNum < g_taskMaxNum; loopNum++) {
        taskCB = (((LosTaskCB *)g_taskCBArray) + loopNum);
        if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
            continue;
        }

        PrintTskInfo(taskCB);
#if (LOSCFG_TASK_MEM_USED == 1)
        PRINTK("%#10x", g_taskMemUsed[loopNum]);
#endif

#if (LOSCFG_BASE_CORE_CPUP == 1)
        PRINTK("%6u.%-2u%7u.%-2u%6u.%-2u ",
               cpuLessOneSec[taskCB->taskID].uwUsage / LOS_CPUP_PRECISION_MULT,
               cpuLessOneSec[taskCB->taskID].uwUsage % LOS_CPUP_PRECISION_MULT,
               cpuTenSec[taskCB->taskID].uwUsage / LOS_CPUP_PRECISION_MULT,
               cpuTenSec[taskCB->taskID].uwUsage % LOS_CPUP_PRECISION_MULT,
               cpuOneSec[taskCB->taskID].uwUsage / LOS_CPUP_PRECISION_MULT,
               cpuOneSec[taskCB->taskID].uwUsage % LOS_CPUP_PRECISION_MULT);
#endif /* LOSCFG_BASE_CORE_CPUP */
#if defined(LOSCFG_KERNEL_SMP)
        PRINTK(" %#4x %3d ", taskCB->cpuAffiMask, taskCB->currCpu);
#endif /* LOSCFG_KERNEL_SMP */
        PRINTK("%#10x %-32s\n", (UINT32)(UINTPTR)taskCB->taskEntry, taskCB->taskName);
    }

#if (LOSCFG_BASE_CORE_CPUP == 1)
    (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, cpuLessOneSec);
    (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, cpuTenSec);
    (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, cpuOneSec);
#endif
#endif
    return LOS_OK;
}

/*****************************************************************************
 Function    : OsTaskInit
 Description : Task init function.
 Input       : None
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsTaskInit(VOID)
{
    UINT32 size;
    UINT32 index;
    UINT32 cpuID;

    g_taskMaxNum = LOSCFG_BASE_CORE_TSK_LIMIT + LOSCFG_KERNEL_CORE_NUM; /* Reserved 1 for IDLE */
    size = (g_taskMaxNum + LOSCFG_KERNEL_CORE_NUM) * sizeof(LosTaskCB);
    g_taskCBArray = (LosTaskCB *)LOS_MemAlloc(m_aucSysMem0, size);
    if (g_taskCBArray == NULL) {
        return LOS_ERRNO_TSK_NO_MEMORY;
    }

    // Ignore the return code when matching CSEC rule 6.6(1).
    (VOID)memset_s(g_taskCBArray, size, 0, size);
    LOS_ListInit(&g_losFreeTask);
    LOS_ListInit(&g_taskRecycleList);
    for (index = 0; index < g_taskMaxNum; index++) {
        g_taskCBArray[index].taskStatus = OS_TASK_STATUS_UNUSED;
        g_taskCBArray[index].taskID = index;
        LOS_ListTailInsert(&g_losFreeTask, &g_taskCBArray[index].pendList);
    }

    // Ignore the return code when matching CSEC rule 6.6(4).
    (VOID)memset_s((VOID *)(&g_losTask), sizeof(g_losTask), 0, sizeof(g_losTask));

    for (cpuID = 0; cpuID < LOSCFG_KERNEL_CORE_NUM; cpuID++) {
        g_losTask[cpuID].runTask = &g_taskCBArray[g_taskMaxNum + cpuID];
        g_losTask[cpuID].runTask->taskID = g_taskMaxNum + cpuID;
        g_losTask[cpuID].runTask->taskStatus = (OS_TASK_STATUS_UNUSED | OS_TASK_STATUS_RUNNING);
        g_losTask[cpuID].runTask->priority = OS_TASK_PRIORITY_LOWEST + 1;
        
        g_idleTaskID[cpuID] = OS_INVALID;
    }
    return OsSchedInit();
}


/*****************************************************************************
 Function    : OsIdleTaskCreate
 Description : Create idle task.
 Input       : cpuID     --- Current CPU ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsIdleTaskCreate(UINT32 cpuID)
{
    UINT32 retVal;
    TSK_INIT_PARAM_S taskInitParam;
    // Ignore the return code when matching CSEC rule 6.6(4).
    (VOID)memset_s((VOID *)(&taskInitParam), sizeof(TSK_INIT_PARAM_S), 0, sizeof(TSK_INIT_PARAM_S));
    taskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)OsIdleTask;
    taskInitParam.uwStackSize = LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE;
    taskInitParam.pcName = "IdleCore000";
    taskInitParam.usTaskPrio = OS_TASK_PRIORITY_LOWEST;
#ifdef LOSCFG_KERNEL_SMP
    taskInitParam.usCpuAffiMask = CPUID_TO_AFFI_MASK(cpuID);
#endif
    retVal = LOS_TaskCreateOnly(&g_idleTaskID[cpuID], &taskInitParam);
    if (retVal != LOS_OK) {
        return retVal;
    }
    OsSchedSetIdleTaskSchedParam(OS_TCB_FROM_TID(g_idleTaskID[cpuID]));

    return LOS_OK;
}

LITE_OS_SEC_TEXT BOOL OsIsIdleTask(UINT32 taskID)
{
    UINT32 cpuID;
    for (cpuID = 0; cpuID < LOSCFG_KERNEL_CORE_NUM; cpuID++) {
        if (taskID == g_idleTaskID[cpuID]) {
            return TRUE;
        }
    }
    return FALSE;
}

/*****************************************************************************
 Function    : LOS_CurTaskIDGet
 Description : get id of current running task.
 Input       : None
 Output      : None
 Return      : task id
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_CurTaskIDGet(VOID)
{
    UINT32 cpuID = ArchCurrCpuid();
    if (g_losTask[cpuID].runTask == NULL) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }
    return g_losTask[cpuID].runTask->taskID;
}

/*****************************************************************************
 Function    : LOS_NextTaskIDGet
 Description : get id of next running task.
 Input       : None
 Output      : None
 Return      : task id
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_NextTaskIDGet(VOID)
{
    UINT32 intSave;
    SCHEDULER_LOCK(intSave);
    UINT32 taskID = OsGetTopTask()->taskID;
    SCHEDULER_UNLOCK(intSave);

    return taskID;
}

/*****************************************************************************
 Function    : LOS_CurTaskNameGet
 Description : get name of current running task.
 Input       : None
 Output      : None
 Return      : task name
 *****************************************************************************/
LITE_OS_SEC_TEXT CHAR *LOS_CurTaskNameGet(VOID)
{
    CHAR *taskName = NULL;
    UINT32 cpuID = ArchCurrCpuid();

    if (g_losTask[cpuID].runTask != NULL) {
        taskName = g_losTask[cpuID].runTask->taskName;
    }

    return taskName;
}

#if (LOSCFG_BASE_CORE_TSK_MONITOR == 1)
#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 0)
/*****************************************************************************
 Function    : OsHandleRunTaskStackOverflow
 Description : handle stack overflow exception of the run task.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT STATIC VOID OsHandleRunTaskStackOverflow(VOID)
{
    UINT32 cpuID = ArchCurrCpuid();
    PRINT_ERR("CURRENT task ID: %s:%d stack overflow!\n",
              g_losTask[cpuID].runTask->taskName, g_losTask[cpuID].runTask->taskID);
    OsDoExcHook(EXC_STACKOVERFLOW);
}

/*****************************************************************************
 Function    : OsHandleNewTaskStackOverflow
 Description : handle stack overflow exception of the new task.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT STATIC VOID OsHandleNewTaskStackOverflow(VOID)
{
    LosTaskCB *tmp = NULL;
    UINT32 cpuID = ArchCurrCpuid();

    PRINT_ERR("HIGHEST task ID: %s:%d SP error!\n",
              g_losTask[cpuID].newTask->taskName, g_losTask[cpuID].newTask->taskID);
    PRINT_ERR("HIGHEST task StackPointer: 0x%x TopOfStack: 0x%x\n",
              (UINT32)(UINTPTR)(g_losTask[cpuID].newTask->stackPointer), g_losTask[cpuID].newTask->topOfStack);

    /*
     * make sure LOS_CurTaskIDGet and LOS_CurTaskNameGet returns the ID and name of which task
     * that occurred stack overflow exception in OsDoExcHook temporary.
     */
    tmp = g_losTask[cpuID].runTask;
    g_losTask[cpuID].runTask = g_losTask[cpuID].newTask;
    OsDoExcHook(EXC_STACKOVERFLOW);
    g_losTask[cpuID].runTask = tmp;
}
#else
LITE_OS_SEC_TEXT STATIC VOID OsTaskStackProtect(VOID)
{
    MPU_CFG_PARA mpuAttr = {0};
    STATIC INT32 id = -1;
    UINT32 cpuID = ArchCurrCpuid();
    if (id == -1) {
        id = ArchMpuUnusedRegionGet();
        if (id < 0) {
            PRINT_ERR("%s %d, get unused id failed!\n", __FUNCTION__, __LINE__);
            return;
        }
    }

    mpuAttr.baseAddr = g_losTask[cpuID].newTask->topOfStack - OS_TASK_STACK_PROTECT_SIZE;
    mpuAttr.size = OS_TASK_STACK_PROTECT_SIZE;
    mpuAttr.memType = MPU_MEM_ON_CHIP_RAM;
    mpuAttr.executable = MPU_NON_EXECUTABLE;
    mpuAttr.shareability = MPU_NO_SHARE;
    mpuAttr.permission = MPU_RO_BY_PRIVILEGED_ONLY;

    ArchMpuDisable();
    (VOID)ArchMpuDisableRegion(id);
    (VOID)ArchMpuSetRegion(id, &mpuAttr);
    ArchMpuEnable(1);
}
#endif
#endif

/*****************************************************************************
 Function    : OsTaskSwitchCheck
 Description : Check task switch
 Input       : Node
 Output      : None
 Return      : None
 *****************************************************************************/
#if (LOSCFG_BASE_CORE_TSK_MONITOR == 1)
LITE_OS_SEC_TEXT VOID OsTaskSwitchCheck(VOID)
{
    UINT32 intSave;
    UINT32 cpuID = ArchCurrCpuid();
    
    SCHEDULER_LOCK(intSave);
#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 0)
    UINT32 endOfStack = g_losTask[cpuID].newTask->topOfStack + g_losTask[cpuID].newTask->stackSize;

    if ((*(UINT32 *)(UINTPTR)(g_losTask[cpuID].runTask->topOfStack)) != OS_TASK_MAGIC_WORD) {
        OsHandleRunTaskStackOverflow();
    }
    if (((UINT32)(UINTPTR)(g_losTask[cpuID].newTask->stackPointer) <= (g_losTask[cpuID].newTask->topOfStack)) ||
        ((UINT32)(UINTPTR)(g_losTask[cpuID].newTask->stackPointer) > endOfStack)) {
        OsHandleNewTaskStackOverflow();
    }
#else
    OsTaskStackProtect();
#endif

#if (LOSCFG_BASE_CORE_EXC_TSK_SWITCH == 1)
    /* record task switch info */
    g_taskSwitchInfo.pid[g_taskSwitchInfo.idx] = (UINT16)(g_losTask[cpuID].newTask->taskID);

    errno_t ret = memcpy_s(g_taskSwitchInfo.name[g_taskSwitchInfo.idx], LOS_TASK_NAMELEN,
                           g_losTask[cpuID].newTask->taskName, LOS_TASK_NAMELEN);
    if (ret != EOK) {
        PRINT_ERR("exc task switch copy file name failed!\n");
    }
    g_taskSwitchInfo.name[g_taskSwitchInfo.idx][LOS_TASK_NAMELEN - 1] = '\0';

    if (++g_taskSwitchInfo.idx == OS_TASK_SWITCH_INFO_COUNT) {
        g_taskSwitchInfo.idx = 0;
        g_taskSwitchInfo.cntInfo.isFull = TRUE;
    }
#endif

    LOSCFG_BASE_CORE_TSK_SWITCH_HOOK();

#if (LOSCFG_BASE_CORE_CPUP == 1)
    OsTskCycleEndStart();
#endif /* LOSCFG_BASE_CORE_CPUP */
    SCHEDULER_UNLOCK(intSave);
}

LITE_OS_SEC_TEXT_MINOR VOID OsTaskMonInit(VOID)
{
#if (LOSCFG_BASE_CORE_EXC_TSK_SWITCH == 1)
    // Ignore the return code when matching CSEC rule 6.6(4).
    (VOID)memset_s(&g_taskSwitchInfo, sizeof(TaskSwitchInfo), 0, sizeof(TaskSwitchInfo));
    g_taskSwitchInfo.cntInfo.maxCnt = OS_TASK_SWITCH_INFO_COUNT;
#endif
    return;
}
#endif

/*****************************************************************************
 Function    : OsTaskEntry
 Description : All task entry
 Input       : taskID     --- The ID of the task to be run
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT VOID OsTaskEntry(UINT32 taskID)
{
    UINT32 retVal;
    LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);

    taskCB->joinRetval = (UINTPTR)taskCB->taskEntry(taskCB->arg);
    retVal = LOS_TaskDelete(taskCB->taskID);
    if (retVal != LOS_OK) {
        PRINT_ERR("Delete Task[TID: %d] Failed!\n", taskCB->taskID);
    }
}

LITE_OS_SEC_TEXT_INIT STATIC_INLINE UINT32 OsTaskInitParamCheck(TSK_INIT_PARAM_S *taskInitParam)
{
    if (taskInitParam == NULL) {
        return LOS_ERRNO_TSK_PTR_NULL;
    }

    if (taskInitParam->pcName == NULL) {
        return LOS_ERRNO_TSK_NAME_EMPTY;
    }

    if (taskInitParam->pfnTaskEntry == NULL) {
        return LOS_ERRNO_TSK_ENTRY_NULL;
    }

    if ((taskInitParam->usTaskPrio) > OS_TASK_PRIORITY_LOWEST) {
        return LOS_ERRNO_TSK_PRIOR_ERROR;
    }

    if (((taskInitParam->usTaskPrio) == OS_TASK_PRIORITY_LOWEST)
        && (taskInitParam->pfnTaskEntry != OS_IDLE_TASK_ENTRY)) {
        return LOS_ERRNO_TSK_PRIOR_ERROR;
    }

    if (taskInitParam->uwStackSize > LOSCFG_SYS_HEAP_SIZE) {
        return LOS_ERRNO_TSK_STKSZ_TOO_LARGE;
    }

    if (taskInitParam->uwStackSize == 0) {
        taskInitParam->uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    }

    if (taskInitParam->uwStackSize < LOSCFG_BASE_CORE_TSK_MIN_STACK_SIZE) {
        return LOS_ERRNO_TSK_STKSZ_TOO_SMALL;
    }
    return LOS_OK;
}

STATIC UINT32 OsNewTaskInit(LosTaskCB *taskCB, TSK_INIT_PARAM_S *taskInitParam)
{
    taskCB->arg             = taskInitParam->uwArg;
    taskCB->stackSize       = taskInitParam->uwStackSize;
    taskCB->taskSem         = NULL;
    taskCB->taskMux         = NULL;
    taskCB->taskStatus      = OS_TASK_STATUS_SUSPEND;
    taskCB->priority        = taskInitParam->usTaskPrio;
    taskCB->timeSlice       = 0;
    taskCB->waitTimes       = 0;
    taskCB->taskEntry       = taskInitParam->pfnTaskEntry;
    taskCB->event.uwEventID = OS_NULL_INT;
    taskCB->eventMask       = 0;
    taskCB->taskName        = taskInitParam->pcName;
    taskCB->msg             = NULL;
#if (LOSCFG_KERNEL_SIGNAL == 1)
    taskCB->sig             = NULL;
#endif
#if defined(LOSCFG_KERNEL_SMP)
    taskCB->cpuAffiMask = (taskInitParam->usCpuAffiMask) ? (taskInitParam->usCpuAffiMask) : LOSCFG_KERNEL_CPU_MASK;
    taskCB->currCpu     = OS_TASK_INVALID_CPUID;
#endif

    SET_SORTLIST_VALUE(&taskCB->sortList, OS_SORT_LINK_INVALID_TIME);
    LOS_EventInit(&(taskCB->event));

    if (taskInitParam->uwResved & LOS_TASK_ATTR_JOINABLE) {
        taskCB->taskStatus |= OS_TASK_FLAG_JOINABLE;
        LOS_ListInit(&taskCB->joinList);
    }

    if (taskInitParam->stackAddr == (UINTPTR)NULL) {
        taskCB->stackSize = ALIGN(taskInitParam->uwStackSize, OS_TASK_STACK_ADDR_ALIGN);
#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 1)
        UINT32 stackSize = taskCB->stackSize + OS_TASK_STACK_PROTECT_SIZE;
        UINTPTR stackPtr = (UINTPTR)LOS_MemAllocAlign(OS_TASK_STACK_ADDR, stackSize, OS_TASK_STACK_PROTECT_SIZE);
        taskCB->topOfStack = stackPtr + OS_TASK_STACK_PROTECT_SIZE;
#else
        taskCB->topOfStack = (UINTPTR)LOS_MemAllocAlign(OS_TASK_STACK_ADDR, taskCB->stackSize,
                                                        LOSCFG_STACK_POINT_ALIGN_SIZE);
#endif
        if (taskCB->topOfStack == (UINTPTR)NULL) {
            return LOS_ERRNO_TSK_NO_MEMORY;
        }
        taskCB->taskStatus |= OS_TASK_FLAG_STACK_FREE;
    } else {
        taskCB->topOfStack = LOS_Align(taskInitParam->stackAddr, LOSCFG_STACK_POINT_ALIGN_SIZE);
        taskCB->stackSize = taskInitParam->uwStackSize - (taskCB->topOfStack - taskInitParam->stackAddr);
        taskCB->stackSize = TRUNCATE(taskCB->stackSize, OS_TASK_STACK_ADDR_ALIGN);
    }

    /* initialize the task stack, write magic num to stack top */
    (VOID)memset_s((VOID *)taskCB->topOfStack, taskCB->stackSize,
                   (INT32)(OS_TASK_STACK_INIT & 0xFF), taskCB->stackSize);

    *((UINT32 *)taskCB->topOfStack) = OS_TASK_MAGIC_WORD;
    taskCB->stackPointer = ArchTskStackInit(taskCB->taskID, taskCB->stackSize, (VOID *)taskCB->topOfStack);
    return LOS_OK;
}

/*****************************************************************************
 Function    : LOS_TaskCreateOnly
 Description : Create a task and suspend
 Input       : taskInitParam --- Task init parameters
 Output      : taskID        --- Save task ID
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskCreateOnly(UINT32 *taskID, TSK_INIT_PARAM_S *taskInitParam)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT32 retVal;

    if (taskID == NULL) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    retVal = OsTaskInitParamCheck(taskInitParam);
    if (retVal != LOS_OK) {
        return retVal;
    }

    OsRecycleFinishedTask();

    SCHEDULER_LOCK(intSave);
    if (LOS_ListEmpty(&g_losFreeTask)) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_TCB_UNAVAILABLE;
    }

    taskCB = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&g_losFreeTask));
    LOS_ListDelete(LOS_DL_LIST_FIRST(&g_losFreeTask));
    SCHEDULER_UNLOCK(intSave);

    retVal = OsNewTaskInit(taskCB, taskInitParam);
    if (retVal != LOS_OK) {
        SCHEDULER_LOCK(intSave);
        OsInsertTCBToFreeList(taskCB);
        SCHEDULER_UNLOCK(intSave);
        return retVal;
    }

    LOSCFG_TASK_CREATE_EXTENSION_HOOK(taskCB);

#if (LOSCFG_BASE_CORE_CPUP == 1)
    SCHEDULER_LOCK(intSave);
    g_cpup[taskCB->taskID].cpupID = taskCB->taskID;
    g_cpup[taskCB->taskID].status = taskCB->taskStatus;
    SCHEDULER_UNLOCK(intSave);
#endif
    *taskID = taskCB->taskID;
    OsHookCall(LOS_HOOK_TYPE_TASK_CREATE, taskCB);
    return retVal;
}

/*****************************************************************************
 Function    : LOS_TaskCreate
 Description : Create a task
 Input       : taskInitParam --- Task init parameters
 Output      : taskID        --- Save task ID
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskCreate(UINT32 *taskID, TSK_INIT_PARAM_S *taskInitParam)
{
    UINT32 retVal;
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;

    retVal = LOS_TaskCreateOnly(taskID, taskInitParam);
    if (retVal != LOS_OK) {
        return retVal;
    }
    taskCB = OS_TCB_FROM_TID(*taskID);

    SCHEDULER_LOCK(intSave);

    OsSchedTaskEnQueue(taskCB);
    SCHEDULER_UNLOCK(intSave);

#ifdef LOSCFG_KERNEL_SMP
    LOS_MpSchedule(OS_MP_CPU_ALL);
#endif
    if (OS_SCHEDULER_ACTIVE) {
        LOS_Schedule();
    }

    return LOS_OK;
}

/*****************************************************************************
 Function    : LOS_TaskResume
 Description : Resume suspend task
 Input       : taskID --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskResume(UINT32 taskID)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT16 tempStatus;
    UINT32 retErr = OS_ERROR;
    BOOL needSched = FALSE;

    if (!OS_TASK_ID_CHECK(taskID)) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    SCHEDULER_LOCK(intSave);
    tempStatus = taskCB->taskStatus;

    if (tempStatus & OS_TASK_STATUS_UNUSED) {
        retErr = LOS_ERRNO_TSK_NOT_CREATED;
        OS_GOTO_ERREND();
    } else if (!(tempStatus & OS_TASK_STATUS_SUSPEND)) {
        retErr = LOS_ERRNO_TSK_NOT_SUSPENDED;
        OS_GOTO_ERREND();
    }

    needSched = OsSchedResume(taskCB);
    if (needSched && OS_SCHEDULER_ACTIVE) {
        SCHEDULER_UNLOCK(intSave);
#ifdef LOSCFG_KERNEL_SMP
        LOS_MpSchedule(OS_MP_CPU_ALL);
#endif
        LOS_Schedule();
        return LOS_OK;
    }

    SCHEDULER_UNLOCK(intSave);
    return LOS_OK;

LOS_ERREND:
    SCHEDULER_UNLOCK(intSave);
    return retErr;
}

/*****************************************************************************
 Function    : LOS_TaskSuspend
 Description : Suspend task
 Input       : taskID --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskSuspend(UINT32 taskID)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT16 tempStatus;
    UINT32 retErr;
    UINT32 cpuID = ArchCurrCpuid();

    retErr = OsCheckTaskIDValid(taskID);
    if (retErr != LOS_OK) {
        return retErr;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    SCHEDULER_LOCK(intSave);
    tempStatus = taskCB->taskStatus;

    if (tempStatus & OS_TASK_STATUS_UNUSED) {
        retErr = LOS_ERRNO_TSK_NOT_CREATED;
        OS_GOTO_ERREND();
    }

    if (tempStatus & OS_TASK_FLAG_SYSTEM_TASK) {
        retErr = LOS_ERRNO_TSK_OPERATE_SYSTEM_TASK;
        OS_GOTO_ERREND();
    }

    if (tempStatus & OS_TASK_STATUS_SUSPEND) {
        retErr = LOS_ERRNO_TSK_ALREADY_SUSPENDED;
        OS_GOTO_ERREND();
    }

    if ((tempStatus & OS_TASK_STATUS_RUNNING) && (g_losTaskLock[cpuID] != 0)) {
        retErr = LOS_ERRNO_TSK_SUSPEND_LOCKED;
        OS_GOTO_ERREND();
    }

    OsSchedSuspend(taskCB);

    if (taskID == g_losTask[cpuID].runTask->taskID) {
        SCHEDULER_UNLOCK(intSave);
        LOS_Schedule();
        return LOS_OK;
    }

    SCHEDULER_UNLOCK(intSave);
    return LOS_OK;

LOS_ERREND:
    SCHEDULER_UNLOCK(intSave);
    return retErr;
}

STATIC VOID OsTaskJoinPostUnsafe(LosTaskCB *taskCB)
{
    LosTaskCB *resumedTask = NULL;

    if (taskCB->taskStatus & OS_TASK_FLAG_JOINABLE) {
        if (!LOS_ListEmpty(&taskCB->joinList)) {
            resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&(taskCB->joinList)));
            OsSchedTaskWake(resumedTask);
        }
    }
}

STATIC UINT32 OsTaskJoinPendUnsafe(LosTaskCB *taskCB)
{
    if (taskCB->taskStatus & OS_TASK_STATUS_EXIT) {
        return LOS_OK;
    } else if ((taskCB->taskStatus & OS_TASK_FLAG_JOINABLE) && LOS_ListEmpty(&taskCB->joinList)) {
        OsSchedTaskWait(&taskCB->joinList, LOS_WAIT_FOREVER);
        return LOS_OK;
    }

    return LOS_NOK;
}

STATIC UINT32 OsTaskSetDetachUnsafe(LosTaskCB *taskCB)
{
    if (taskCB->taskStatus & OS_TASK_FLAG_JOINABLE) {
        if (LOS_ListEmpty(&(taskCB->joinList))) {
            LOS_ListDelete(&(taskCB->joinList));
            taskCB->taskStatus &= ~OS_TASK_FLAG_JOINABLE;
            return LOS_OK;
        }
        /* This error code has a special purpose and is not allowed to appear again on the interface */
        return LOS_ERRNO_TSK_NOT_JOIN;
    }

    return LOS_NOK;
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskJoin(UINT32 taskID, UINTPTR *retval)
{
    LosTaskCB *taskCB = NULL;
    UINTPTR stackPtr = 0;
    UINT32 intSave;
    UINT32 ret;
    UINT32 cpuID = ArchCurrCpuid();

    ret = OsCheckTaskIDValid(taskID);
    if (ret != LOS_OK) {
        return ret;
    }

    if (OS_INT_ACTIVE) {
        return LOS_ERRNO_TSK_NOT_ALLOW_IN_INT;
    }

    if (g_losTaskLock[cpuID] != 0) {
        return LOS_ERRNO_TSK_SCHED_LOCKED;
    }

    if (taskID == LOS_CurTaskIDGet()) {
        return LOS_ERRNO_TSK_NOT_JOIN_SELF;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    SCHEDULER_LOCK(intSave);
    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    ret = OsTaskJoinPendUnsafe(taskCB);
    SCHEDULER_UNLOCK(intSave);
    if (ret == LOS_OK) {
        LOS_Schedule();

        if (retval != NULL) {
            *retval = taskCB->joinRetval;
        }

        SCHEDULER_LOCK(intSave);
        taskCB->taskStatus &= ~OS_TASK_FLAG_JOINABLE;
        OsRecycleTaskResources(taskCB, &stackPtr);
        SCHEDULER_UNLOCK(intSave);
        (VOID)LOS_MemFree(OS_TASK_STACK_ADDR, (VOID *)stackPtr);
        return LOS_OK;
    }

    return ret;
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskDetach(UINT32 taskID)
{
    UINT32 intSave;
    UINT32 ret;
    LosTaskCB *taskCB = NULL;

    ret = OsCheckTaskIDValid(taskID);
    if (ret != LOS_OK) {
        return ret;
    }

    if (OS_INT_ACTIVE) {
        return LOS_ERRNO_TSK_NOT_ALLOW_IN_INT;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    SCHEDULER_LOCK(intSave);
    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    if (taskCB->taskStatus & OS_TASK_STATUS_EXIT) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_TaskJoin(taskID, NULL);
    }

    ret = OsTaskSetDetachUnsafe(taskCB);
    SCHEDULER_UNLOCK(intSave);
    return ret;
}

LITE_OS_SEC_TEXT_INIT STATIC_INLINE VOID OsRunningTaskDelete(UINT32 taskID, LosTaskCB *taskCB)
{
    UINT32 cpuID = ArchCurrCpuid();
    LOS_ListTailInsert(&g_taskRecycleList, &taskCB->pendList);
    g_losTask[cpuID].runTask = &g_taskCBArray[g_taskMaxNum + cpuID];
    g_losTask[cpuID].runTask->taskID = taskID;
    g_losTask[cpuID].runTask->taskStatus = taskCB->taskStatus | OS_TASK_STATUS_RUNNING;
    g_losTask[cpuID].runTask->topOfStack = taskCB->topOfStack;
    g_losTask[cpuID].runTask->taskName = taskCB->taskName;
}
/*****************************************************************************
 Function    : LOS_TaskDelete
 Description : Delete a task
 Input       : taskID --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskDelete(UINT32 taskID)
{
    UINT32 intSave;
    UINTPTR stackPtr = 0;
    LosTaskCB *taskCB = NULL;
    UINT32 cpuID = ArchCurrCpuid();

    UINT32 ret = OsCheckTaskIDValid(taskID);
    if (ret != LOS_OK) {
        return ret;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    SCHEDULER_LOCK(intSave);
    if (taskCB->taskStatus & OS_TASK_FLAG_SYSTEM_TASK) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_OPERATE_SYSTEM_TASK;
    }

    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    if (taskCB->taskStatus & OS_TASK_STATUS_EXIT) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_ALREADY_EXIT;
    }

    if (taskCB->taskStatus & OS_TASK_FLAG_SIGNAL) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_PROCESS_SIGNAL;
    }

    /* If the task is running and scheduler is locked then you can not delete it */
    if (((taskCB->taskStatus) & OS_TASK_STATUS_RUNNING) && (g_losTaskLock[cpuID] != 0)) {
        PRINT_INFO("In case of task lock, task deletion is not recommended\n");
        g_losTaskLock[cpuID] = 0;
    }

    OsHookCall(LOS_HOOK_TYPE_TASK_DELETE, taskCB);
    OsTaskJoinPostUnsafe(taskCB);
    OsSchedTaskExit(taskCB);

    LOS_EventDestroy(&(taskCB->event));
    taskCB->event.uwEventID = OS_NULL_INT;
    taskCB->eventMask = 0;
#if (LOSCFG_BASE_CORE_CPUP == 1)
    // Ignore the return code when matching CSEC rule 6.6(4).
    (VOID)memset_s((VOID *)&g_cpup[taskCB->taskID], sizeof(OsCpupCB), 0, sizeof(OsCpupCB));
#endif

#if (LOSCFG_KERNEL_SIGNAL == 1)
    if (taskCB->sig != NULL) {
        LOS_MemFree(OS_SYS_MEM_ADDR, taskCB->sig);
        taskCB->sig = NULL;
    }
#endif

    LOSCFG_TASK_DELETE_EXTENSION_HOOK(taskCB);

    if (taskCB->taskStatus & OS_TASK_STATUS_RUNNING) {
        if (!(taskCB->taskStatus & OS_TASK_FLAG_JOINABLE)) {
            taskCB->taskStatus |= OS_TASK_STATUS_UNUSED;
            OsRunningTaskDelete(taskID, taskCB);
        }
        SCHEDULER_UNLOCK(intSave);
#ifdef LOSCFG_KERNEL_SMP
        LOS_MpSchedule(CPUID_TO_AFFI_MASK(taskCB->currCpu));
#endif
        LOS_Schedule();
        return LOS_OK;
    }

    taskCB->joinRetval = LOS_CurTaskIDGet();
    OsRecycleTaskResources(taskCB, &stackPtr);
    SCHEDULER_UNLOCK(intSave);
    (VOID)LOS_MemFree(OS_TASK_STACK_ADDR, (VOID *)stackPtr);
    return LOS_OK;
}

/*****************************************************************************
 Function    : LOS_TaskDelay
 Description : delay the current task
 Input       : tick    --- time
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_TaskDelay(UINT32 tick)
{
    UINT32 intSave;
    UINT32 cpuID = ArchCurrCpuid();

    if (OS_INT_ACTIVE) {
        return LOS_ERRNO_TSK_DELAY_IN_INT;
    }

    if (g_losTaskLock[cpuID] != 0) {
        return LOS_ERRNO_TSK_DELAY_IN_LOCK;
    }

    if (g_losTask[cpuID].runTask->taskStatus & OS_TASK_FLAG_SYSTEM_TASK) {
        return LOS_ERRNO_TSK_OPERATE_SYSTEM_TASK;
    }
    OsHookCall(LOS_HOOK_TYPE_TASK_DELAY, tick);
    if (tick == 0) {
        return LOS_TaskYield();
    } else {
        SCHEDULER_LOCK(intSave);
        OsSchedDelay(g_losTask[cpuID].runTask, tick);
        OsHookCall(LOS_HOOK_TYPE_MOVEDTASKTODELAYEDLIST, g_losTask[cpuID].runTask);
        SCHEDULER_UNLOCK(intSave);
        LOS_Schedule();
    }

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT16 LOS_TaskPriGet(UINT32 taskID)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT16 priority;

    if (OS_CHECK_TSK_PID_NOIDLE(taskID)) {
        return (UINT16)OS_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskID);

    SCHEDULER_LOCK(intSave);

    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return (UINT16)OS_INVALID;
    }

    priority = taskCB->priority;
    SCHEDULER_UNLOCK(intSave);
    return priority;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskPriSet(UINT32 taskID, UINT16 taskPrio)
{
    BOOL isReady = FALSE;
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT16 tempStatus;

    if (taskPrio > OS_TASK_PRIORITY_LOWEST) {
        return LOS_ERRNO_TSK_PRIOR_ERROR;
    }

    if (OsIsIdleTask(taskID)) {
        return LOS_ERRNO_TSK_OPERATE_IDLE;
    }

#if (LOSCFG_BASE_CORE_SWTMR == 1)
    if (taskID == g_swtmrTaskID) {
        return LOS_ERRNO_TSK_OPERATE_SWTMR;
    }
#endif

    if (OS_CHECK_TSK_PID_NOIDLE(taskID)) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    SCHEDULER_LOCK(intSave);
    tempStatus = taskCB->taskStatus;
    if (tempStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }
    if (tempStatus & OS_TASK_FLAG_SYSTEM_TASK) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_OPERATE_SYSTEM_TASK;
    }

    isReady = OsSchedModifyTaskSchedParam(taskCB, taskPrio);
    SCHEDULER_UNLOCK(intSave);
    /* delete the task and insert with right priority into ready queue */
    if (isReady) {
#ifdef LOSCFG_KERNEL_SMP
        LOS_MpSchedule(OS_MP_CPU_ALL);
#endif
        LOS_Schedule();
    }

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_CurTaskPriSet(UINT16 taskPrio)
{
    UINT32 cpuID = ArchCurrCpuid();
    return LOS_TaskPriSet(g_losTask[cpuID].runTask->taskID, taskPrio);
}

/*****************************************************************************
 Function    : LOS_TaskYield
 Description : Adjust the procedure order of specified task
 Input       : None
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskYield(VOID)
{
    UINT32 intSave;

    SCHEDULER_LOCK(intSave);
    OsSchedYield();
    SCHEDULER_UNLOCK(intSave);
    LOS_Schedule();
    return LOS_OK;
}

/*****************************************************************************
 Function    : LOS_TaskLock
 Description : Task lock
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID LOS_TaskLock(VOID)
{
    INT32 cpuID = ArchCurrCpuid();
    UINT32 intSave;

    SCHEDULER_LOCK(intSave);
    g_losTaskLock[cpuID]++;
    SCHEDULER_UNLOCK(intSave);
}

/*****************************************************************************
 Function    : LOS_TaskUnlock
 Description : Task unlock
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID LOS_TaskUnlock(VOID)
{
    UINT32 intSave;
    INT32 cpuID = ArchCurrCpuid();

    SCHEDULER_LOCK(intSave);
    if (g_losTaskLock[cpuID] > 0) {
        g_losTaskLock[cpuID]--;
        if (g_losTaskLock[cpuID] == 0) {
            SCHEDULER_UNLOCK(intSave);
            LOS_Schedule();
            return;
        }
    }

    SCHEDULER_UNLOCK(intSave);
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskInfoGet(UINT32 taskID, TSK_INFO_S *taskInfo)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;

    if (taskInfo == NULL) {
        return LOS_ERRNO_TSK_PTR_NULL;
    }

    if (OS_CHECK_TSK_PID_NOIDLE(taskID)) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    SCHEDULER_LOCK(intSave);

    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    taskInfo->uwSP = (UINT32)(UINTPTR)taskCB->stackPointer;
    taskInfo->usTaskStatus = taskCB->taskStatus;
    taskInfo->usTaskPrio = taskCB->priority;
    taskInfo->uwStackSize = taskCB->stackSize;
    taskInfo->uwTopOfStack = taskCB->topOfStack;
    taskInfo->uwEvent = taskCB->event;
    taskInfo->uwEventMask = taskCB->eventMask;
    taskInfo->uwSemID = (taskCB->taskSem != NULL) ? ((LosSemCB *)(taskCB->taskSem))->semID :
                        LOSCFG_BASE_IPC_SEM_LIMIT;
    taskInfo->uwMuxID = (taskCB->taskMux != NULL) ? ((LosMuxCB *)(taskCB->taskMux))->muxID :
                        LOSCFG_BASE_IPC_MUX_LIMIT;
    taskInfo->pTaskSem = taskCB->taskSem;
    taskInfo->pTaskMux = taskCB->taskMux;
    taskInfo->uwTaskID = taskID;
    // Ignore the return code when matching CSEC rule 6.6(4).
    (VOID)strncpy_s(taskInfo->acName, LOS_TASK_NAMELEN, taskCB->taskName, LOS_TASK_NAMELEN - 1);
    taskInfo->acName[LOS_TASK_NAMELEN - 1] = '\0';

    taskInfo->uwBottomOfStack = TRUNCATE(((UINT32)(taskCB->topOfStack) + (taskCB->stackSize)),
                                         OS_TASK_STACK_ADDR_ALIGN);
    taskInfo->uwCurrUsed = taskInfo->uwBottomOfStack - taskInfo->uwSP;
    taskInfo->uwPeakUsed = OsGetTaskWaterLine(taskID);
    taskInfo->bOvf = (taskInfo->uwPeakUsed == OS_NULL_INT) ? TRUE : FALSE;
    SCHEDULER_UNLOCK(intSave);

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskStatusGet(UINT32 taskID, UINT32 *taskStatus)
{
    UINT32    intSave;
    LosTaskCB *taskCB = NULL;

    if (taskStatus == NULL) {
        return LOS_ERRNO_TSK_PTR_NULL;
    }

    if (OS_CHECK_TSK_PID_NOIDLE(taskID)) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    SCHEDULER_LOCK(intSave);

    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    *taskStatus = taskCB->taskStatus;

    SCHEDULER_UNLOCK(intSave);

    return LOS_OK;
}

#if (LOSCFG_BASE_CORE_EXC_TSK_SWITCH == 1)
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskSwitchInfoGet(UINT32 index, UINT32 *taskSwitchInfo)
{
    UINT32 intSave;
    UINT32 curIndex;

    curIndex = index;
    if (curIndex >= OS_TASK_SWITCH_INFO_COUNT) {
        curIndex %= OS_TASK_SWITCH_INFO_COUNT;
    }

    if (taskSwitchInfo == NULL) {
        return LOS_ERRNO_TSK_PTR_NULL;
    }

    SCHEDULER_LOCK(intSave);

    (*taskSwitchInfo) = g_taskSwitchInfo.pid[curIndex];

    if (memcpy_s((VOID *)(taskSwitchInfo + 1), LOS_TASK_NAMELEN,
                 g_taskSwitchInfo.name[curIndex], LOS_TASK_NAMELEN) != EOK) {
        PRINT_ERR("LOS_TaskSwitchInfoGet copy task name failed\n");
    }

    SCHEDULER_UNLOCK(intSave);
    return LOS_OK;
}
#endif

/*****************************************************************************
Function    : LOS_TaskInfoMonitor
Description : Get all task info
Input       : None
Return      : LOS_OK on success ,or OS_ERROR on failure
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskInfoMonitor(VOID)
{
    UINT32 retVal;

    retVal = OsGetAllTskInfo();

    return retVal;
}

/*****************************************************************************
 Function    : LOS_TaskIsRunning
 Description : Check if LiteOS has been started.
 Input       : VOID
 Output      : VOID
 Return      : TRUE means LiteOS was started, FALSE means not.
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR BOOL LOS_TaskIsRunning(VOID)
{
    return OS_SCHEDULER_ACTIVE;
}

/*****************************************************************************
 Function    : LOS_NewTaskIDGet
 Description : get id of current new task.
 Input       : None
 Output      : None
 Return      : task id
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_NewTaskIDGet(VOID)
{
    return LOS_NextTaskIDGet();
}

/*****************************************************************************
 Function    : LOS_TaskNameGet
 Description : get Name of current new task.
 Input       : taskID -----task id
 Output      : None
 Return      : task name
 *****************************************************************************/
LITE_OS_SEC_TEXT CHAR* LOS_TaskNameGet(UINT32 taskID)
{
    UINT32    intSave;
    LosTaskCB *taskCB = NULL;

    if (OS_CHECK_TSK_PID_NOIDLE(taskID)) {
        return NULL;
    }

    taskCB = OS_TCB_FROM_TID(taskID);

    SCHEDULER_LOCK(intSave);
    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return NULL;
    }
    SCHEDULER_UNLOCK(intSave);

    return taskCB->taskName;
}

LITE_OS_SEC_TEXT_MINOR VOID LOS_Msleep(UINT32 mSecs)
{
    UINT32 interval;

    if (OS_INT_ACTIVE) {
        return;
    }

    if (mSecs == 0) {
        interval = 0;
    } else {
        interval = LOS_MS2Tick(mSecs);
        if (interval == 0) {
            interval = 1;
        }
    }

    (VOID)LOS_TaskDelay(interval);
}

VOID LOS_TaskResRecycle(VOID)
{
    OsRecycleFinishedTask();
}

LITE_OS_SEC_TEXT BOOL OsTaskCpuAffiSetUnsafe(UINT32 taskID, UINT16 newCpuAffiMask, UINT16 *oldCpuAffiMask)
{
#ifdef LOSCFG_KERNEL_SMP
    LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);

    taskCB->cpuAffiMask = newCpuAffiMask;
    *oldCpuAffiMask = CPUID_TO_AFFI_MASK(taskCB->currCpu);
    if (!((*oldCpuAffiMask) & newCpuAffiMask)) {
        return TRUE;
    }
#else
    (VOID)taskID;
    (VOID)newCpuAffiMask;
    (VOID)oldCpuAffiMask;
#endif /* LOSCFG_KERNEL_SMP */
    return FALSE;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskCpuAffiSet(UINT32 taskID, UINT16 cpuAffiMask)
{
    BOOL needSched = FALSE;
    UINT32 intSave;
    UINT16 currCpuMask;

    if (OS_TASK_ID_CHECK(taskID)) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    if (!(cpuAffiMask & LOSCFG_KERNEL_CPU_MASK)) {
        return LOS_ERRNO_TSK_CPU_AFFINITY_MASK_ERR;
    }

    LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);
    SCHEDULER_LOCK(intSave);
    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }
    needSched = OsTaskCpuAffiSetUnsafe(taskID, cpuAffiMask, &currCpuMask);

    SCHEDULER_UNLOCK(intSave);
    if (needSched && OS_SCHEDULER_ACTIVE) {
#ifdef LOSCFG_KERNEL_SMP
        LOS_MpSchedule(currCpuMask);
#endif
        LOS_Schedule();
    }

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskCpuAffiGet(UINT32 taskID)
{
#ifdef LOSCFG_KERNEL_SMP
#define INVALID_CPU_AFFI_MASK   0
    UINT32 intSave;
    UINT16 cpuAffiMask;

    if (OS_TASK_ID_CHECK(taskID)) {
        return INVALID_CPU_AFFI_MASK;
    }
    LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);
    SCHEDULER_LOCK(intSave);
    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return INVALID_CPU_AFFI_MASK;
    }

    cpuAffiMask = taskCB->cpuAffiMask;
    SCHEDULER_UNLOCK(intSave);

    return cpuAffiMask;
#else
    (VOID)taskID;
    return 1;
#endif
}


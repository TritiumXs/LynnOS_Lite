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

#ifndef _LOS_SCHED_H
#define _LOS_SCHED_H

#include "los_task.h"
#include "los_interrupt.h"
#include "los_tick.h"
#include "los_sortlink.h"

#include "los_core.h"
#include "los_spinlock.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define OS_PRIORITY_QUEUE_NUM       (32)

#define OS_SCHED_MINI_PERIOD       (g_sysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND_MINI)
#define OS_SCHED_MAX_RESPONSE_TIME OS_SORT_LINK_UINT64_MAX

extern UINT32 g_taskScheduled;
typedef BOOL (*SchedScan)(VOID);

typedef enum {
    SCHED_FLAG_UP = 0,
    SCHED_FLAG_RUN,
} SCHED_FLAG;

#define OS_MP_CPU_ALL           LOSCFG_KERNEL_CPU_MASK

#define BIT(nr)                 (1UL << (nr))
#define LOS_MP_IPI_SCHEDULE     BIT(0)

typedef struct {
    SortLinkAttribute *taskSortLinkList;						/* The timeout task queue.If the task is in a timeout state, it will be added to the queue, and after the timeout is triggered, the task will be woken up for scheduling */
	struct Spinlock   sortLinkLock;
    LOS_DL_LIST 	  priQueueList[OS_PRIORITY_QUEUE_NUM];		/* Priority queue */
    struct Spinlock   priQueueLock;
    UINT16            taskNum;
    UINT16            tickIntLock;                              /* Tick interrupts the lock flag */
	UINT32 			  queueBitmap;								/* Priority bitmap */
	UINT64            responseTime; 							/* Response time for current CPU tick interrupts */
	UINT32            responseID;   							/* The response ID of the current CPU tick interrupt */
	UINT32            idleTaskID;   							/* idle task id */
    UINT32            swtmrTaskID;                              /* software timer task id */
	UINT32            taskLockCnt;  							/* task lock status */
	UINT32            schedFlag;    							/* The scheduling status of the current scheduling queue */
} SchedRunqueue;

extern SchedRunqueue g_schedRunqueue[LOSCFG_KERNEL_CORE_NUM];

// Gets the SchedRunqueue structure for the specified core
STATIC INLINE SchedRunqueue *OsGetRunQueue(INT32 cpuid)
{
	return &(g_schedRunqueue[cpuid]);
}

// Gets the SchedRunqueue structure for the current core
STATIC INLINE SchedRunqueue *OsGetCurrRunQueue(VOID)
{
	return OsGetRunQueue(ArchCurrCpuid());
}

// set rq status to SCHED_FLAG_RUN
#define OS_SCHED_RQ_RUN(cpuid) do { \
    g_schedRunqueue[cpuid].schedFlag = SCHED_FLAG_RUN; \
} while (0)

// set current rq status to SCHED_FLAG_RUN
#define OS_SCHED_CURR_RQ_RUN() do { \
    g_schedRunqueue[ArchCurrCpuid()].schedFlag = SCHED_FLAG_RUN; \
} while (0)

// check rq status is SCHED_FLAG_RUN or not
#define OS_SCHED_RQ_IS_RUNNING(cpuid) \
    (g_schedRunqueue[cpuid].schedFlag == SCHED_FLAG_RUN)

// check current rq status is SCHED_FLAG_RUN or not
#define OS_SCHED_CURR_RQ_IS_RUNNING() \
    (g_schedRunqueue[ArchCurrCpuid()].schedFlag == SCHED_FLAG_RUN)

VOID OsSchedResetSchedResponseTime(UINT64 responseTime);

VOID OsSchedSetIdleTaskSchedParam(LosTaskCB *idleTask);

UINT32 OsSchedSwtmrScanRegister(SchedScan func);

VOID OsSchedUpdateExpireTime(VOID);

UINT64 OsSchedGetNextExpireTime(UINT64 startTime);

VOID OsSchedTaskDeQueue(LosTaskCB *taskCB);

VOID OsSchedTaskEnQueue(LosTaskCB *taskCB);

VOID OsSchedTaskWait(LOS_DL_LIST *list, UINT32 timeout);

VOID OsSchedTaskWake(LosTaskCB *resumedTask);

BOOL OsSchedModifyTaskSchedParam(LosTaskCB *taskCB, UINT16 priority);

VOID OsSchedDelay(LosTaskCB *runTask, UINT32 tick);

VOID OsSchedYield(VOID);

VOID OsSchedTaskExit(LosTaskCB *taskCB);

VOID OsSchedSuspend(LosTaskCB *taskCB);

BOOL OsSchedResume(LosTaskCB *taskCB);

VOID OsSchedTaskReady(LosTaskCB *taskCB);

VOID OsSchedTick(VOID);

UINT32 OsSchedInit(VOID);

VOID OsSchedStart(VOID);

BOOL OsSchedTaskSwitch(VOID);

LosTaskCB *OsGetTopTask(VOID);

VOID OsSchedTimeConvertFreq(UINT32 oldFreq);

STATIC INLINE UINT64 OsGetCurrSchedTimeCycle(VOID)
{
    return LOS_SysCycleGet();
}

STATIC INLINE BOOL OsCheckKernelRunning(VOID)
{
    return (g_taskScheduled && LOS_CHECK_SCHEDULE);
}

/**
 * @ingroup los_sched
 * @brief Get the time, in nanoseconds, remaining before the next tick interrupt response.
 *
 * @par Description:
 * This API is used to get the time, in nanoseconds, remaining before the next tick interrupt response.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval #time, in nanoseconds.
 * @par Dependency:
 * <ul><li>los_sched.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
extern UINT64 LOS_SchedTickTimeoutNsGet(VOID);

/**
 * @ingroup los_sched
 * @brief The system-provided tick interrupt handler.
 *
 * @par Description:
 * This API is used to wake up a task that is blocked by time.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_sched.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
extern VOID LOS_SchedTickHandler(VOID);

/**
 * @ingroup los_sched
 * @brief Trigger a system dispatch.
 *
 * @par Description:
 * This API is used to trigger a system dispatch.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_sched.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
extern VOID LOS_Schedule(VOID);

extern VOID LOS_MpSchedule(UINT32 target);

// check the task id is idle task or not
// extern BOOL OsIsIdleTask(UINT32 taskid);

VOID OsRunQueueRegisterIdleTask(INT32 cpuid, UINT32 taskid);

// check the task id is swtmr task or not
extern BOOL OsIsSWTmrTask(UINT32 taskid);

VOID OsRunQueueRegisterSWTmrTask(INT32 cpuid, UINT32 taskid);

// get current core idle task id
STATIC INLINE UINT32 OsGetIdleTaskID(INT32 cpuid)
{
    SchedRunqueue *rq = OsGetRunQueue(cpuid);
    return rq->idleTaskID;
}

// get current core idle task id
STATIC INLINE UINT32 OsGetCurrIdleTaskID(VOID)
{
    return OsGetIdleTaskID(ArchCurrCpuid());
}

#define OS_SCHED_INVALID_RUNQUEUE_NUM   ((UINT32)-1)
INT32 OsSchedSelectRunQueue(UINT32 cpuAffiMask);

extern VOID OsSchedLock(UINT32 *intSave);

extern VOID OsSchedUnlock(UINT32 intSave);

extern UINT32 OsSchedTimeoutHandle(LosTaskCB *taskCB);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_SCHED_H */

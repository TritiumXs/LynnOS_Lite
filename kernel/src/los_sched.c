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

#include "los_sched.h"
#include "los_task.h"
#include "los_tick.h"
#include "los_swtmr.h"
#include "los_debug.h"
#include "los_hook.h"
#if (LOSCFG_KERNEL_PM == 1)
#include "los_pm.h"
#endif
#if (LOSCFG_DEBUG_TOOLS == 1)
#include "los_debugtools.h"
#endif
#include "los_spinlock.h"
#if (LOSCFG_SOC_COMPANY_ESPRESSIF == 1)
#include "soc/spinlock.h"
#endif

#define PRIQUEUE_PRIOR0_BIT        0x80000000U
#define OS_TICK_RESPONSE_TIME_MAX  LOSCFG_BASE_CORE_TICK_RESPONSE_MAX
#if (LOSCFG_BASE_CORE_TICK_RESPONSE_MAX == 0)
#error "Must specify the maximum value that tick timer counter supports!"
#endif

#define OS_TASK_BLOCKED_STATUS (OS_TASK_STATUS_PEND | OS_TASK_STATUS_SUSPEND | \
                                OS_TASK_STATUS_EXIT | OS_TASK_STATUS_UNUSED)

STATIC SchedScan  g_swtmrScan = NULL;

STATIC UINT64 g_schedResponseTime[LOSCFG_KERNEL_CORE_NUM] = {OS_SCHED_MAX_RESPONSE_TIME};

STATIC INT32 g_schedTimeSlice;
STATIC INT32 g_schedTimeSliceMin;
STATIC UINT32 g_schedTickMinPeriod;
STATIC UINT32 g_tickResponsePrecision;

SchedRunqueue g_schedRunqueue[LOSCFG_KERNEL_CORE_NUM] = {0};

STATIC struct Spinlock g_schedSpinLock = LOS_SPINLOCK_INITIALIZER;

// ready queue locks
#define SCHED_RQ_LOCK(rq, intSave)   LOS_SpinLockSave(&((rq)->priQueueLock), &(intSave))
#define SCHED_RQ_UNLOCK(rq, intSave) LOS_SpinUnlockRestore(&((rq)->priQueueLock), (intSave))

// sortlist lock
#define SCHED_SL_LOCK(rq, intSave)   LOS_SpinLockSave(&((rq)->sortLinkLock), &(intSave))
#define SCHED_SL_UNLOCK(rq, intSave) LOS_SpinUnlockRestore(&((rq)->sortLinkLock), (intSave))

STATIC INLINE UINT32 SchedGetCurrResponseID(VOID)
{
    INT32 cpuid = ArchCurrCpuid();
    return g_schedRunqueue[cpuid].responseID;
}

STATIC INLINE UINT32 SchedGetResponseID(INT32 cpuid)
{
    return g_schedRunqueue[cpuid].responseID;
}

STATIC INLINE VOID SchedSetResponseID(INT32 cpuid, UINT32 responseID)
{
    g_schedRunqueue[cpuid].responseID = responseID;
}

STATIC INLINE VOID SchedSetCurrResponseID(UINT32 responseID)
{
    SchedSetResponseID(ArchCurrCpuid(), responseID);
}

#define SCHED_INT_LOCK() do { \
        g_schedRunqueue[ArchCurrCpuid()].tickIntLock++; \
    } while (0)

#define SCHED_INT_UNLOCK() do { \
        g_schedRunqueue[ArchCurrCpuid()].tickIntLock--; \
    } while (0)

#define SCHED_INT_LOCK_CHECK(cpuid) \
    (g_schedRunqueue[cpuid].tickIntLock == 0)

// Schedule lock lock
VOID OsSchedLock(UINT32 *intSave)
{
    LOS_SpinLockSave(&(g_schedSpinLock), intSave);
}

// Schedule lock unlock
VOID OsSchedUnlock(UINT32 intSave)
{
    LOS_SpinUnlockRestore(&(g_schedSpinLock), intSave);
}

VOID OsSchedResetSchedResponseTime(UINT64 responseTime)
{
    INT32 cpuid = ArchCurrCpuid();
    if (responseTime <= g_schedResponseTime[cpuid]) {
        g_schedResponseTime[cpuid] = OS_SCHED_MAX_RESPONSE_TIME;
    }

}

STATIC INLINE VOID OsTimeSliceUpdate(LosTaskCB *taskCB, UINT64 currTime)
{
    LOS_ASSERT(currTime >= taskCB->startTime);

    INT32 incTime = currTime - taskCB->startTime;
    if (!OsIsIdleTask(taskCB->taskID)) {
        taskCB->timeSlice -= incTime;
    }
    taskCB->startTime = currTime;
}

STATIC INLINE VOID OsSchedSetNextExpireTime(UINT32 responseID, UINT64 taskEndTime)
{
    UINT64 nextResponseTime;
    BOOL isTimeSlice = FALSE;
    INT32 cpuid = ArchCurrCpuid();

    UINT64 currTime = OsGetCurrSchedTimeCycle();
    UINT64 nextExpireTime = OsGetNextExpireTime(currTime, g_tickResponsePrecision, cpuid);
    /* The response time of the task time slice is aligned to the next response time in the delay queue */
    if ((nextExpireTime > taskEndTime) && ((nextExpireTime - taskEndTime) > g_schedTickMinPeriod)) {
        nextExpireTime = taskEndTime;
        isTimeSlice = TRUE;
    }

    if ((g_schedResponseTime[cpuid] <= nextExpireTime)
         || ((g_schedResponseTime[cpuid] - nextExpireTime) < g_tickResponsePrecision)) {
        return;
    }

    if (isTimeSlice) {
        /* The expiration time of the current system is the thread's slice expiration time */
        SchedSetResponseID(cpuid, responseID);
    } else {
        SchedSetResponseID(cpuid, OS_INVALID);
    }

    nextResponseTime = nextExpireTime - currTime;
    if (nextResponseTime < g_tickResponsePrecision) {
        nextResponseTime = g_tickResponsePrecision;
    }
    g_schedResponseTime[cpuid] = currTime + OsTickTimerReload(nextResponseTime);
}

VOID OsSchedUpdateExpireTime(VOID)
{
    INT32 cpuid = ArchCurrCpuid();
    UINT64 endTime;
    BOOL isPmMode = FALSE;
    LosTask *losTask = OsTaskGetByID(cpuid);
    LosTaskCB *runTask = losTask->runTask;
    INT32 timeSlice;

    if (!OS_SCHED_RQ_IS_RUNNING(cpuid) || !SCHED_INT_LOCK_CHECK(cpuid)) {
        return;
    }

#if (LOSCFG_KERNEL_PM == 1)
    isPmMode = OsIsPmMode();
#endif
    if (!OsIsIdleTask(runTask->taskID) && !isPmMode) {
        timeSlice = (runTask->timeSlice <= g_schedTimeSliceMin) ? g_schedTimeSlice : runTask->timeSlice;

        endTime = runTask->startTime + timeSlice;
    } else {
        endTime = OS_SCHED_MAX_RESPONSE_TIME - g_tickResponsePrecision;
    }

    OsSchedSetNextExpireTime(runTask->taskID, endTime);
}

STATIC INLINE VOID OsSchedPriQueueEnHead(SchedRunqueue *rq, LOS_DL_LIST *priqueueItem, UINT32 priority)
{
    if (NULL == rq) return;

    /*
     * Task control blocks are inited as zero. And when task is deleted,
     * and at the same time would be deleted from priority queue or
     * other lists, task pend node will restored as zero.
     */
    if (LOS_ListEmpty(&(rq->priQueueList[priority]))) {
        rq->queueBitmap |= PRIQUEUE_PRIOR0_BIT >> priority;
    }

    LOS_ListAdd(&(rq->priQueueList[priority]), priqueueItem);
}

STATIC INLINE VOID OsSchedPriQueueEnTail(SchedRunqueue *rq, LOS_DL_LIST *priqueueItem, UINT32 priority)
{
    if (NULL == rq) return;

    if (LOS_ListEmpty(&(rq->priQueueList[priority]))) {
        rq->queueBitmap |= PRIQUEUE_PRIOR0_BIT >> priority;
    }

    LOS_ListTailInsert(&(rq->priQueueList[priority]), priqueueItem);
}

STATIC INLINE VOID OsSchedPriQueueDelete(SchedRunqueue *rq, LOS_DL_LIST *priqueueItem, UINT32 priority)
{
    if (NULL == rq) return;

    LOS_ListDelete(priqueueItem);
    if (LOS_ListEmpty(&(rq->priQueueList[priority]))) {
        rq->queueBitmap &= ~(PRIQUEUE_PRIOR0_BIT >> priority);
    }
}

STATIC INLINE VOID OsSchedWakePendTimeTask(LosTaskCB *taskCB, BOOL *needSchedule)
{
    UINT16 tempStatus = taskCB->taskStatus;
    if (tempStatus & (OS_TASK_STATUS_PEND | OS_TASK_STATUS_DELAY)) {
        taskCB->taskStatus &= ~(OS_TASK_STATUS_PEND | OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_DELAY);
        if (tempStatus & OS_TASK_STATUS_PEND) {
            taskCB->taskStatus |= OS_TASK_STATUS_TIMEOUT;
            LOS_ListDelete(&taskCB->pendList);
            taskCB->taskMux = NULL;
            taskCB->taskSem = NULL;
        }

        if (!(tempStatus & OS_TASK_STATUS_SUSPEND)) {
            OsSchedTaskEnQueue(taskCB);
            *needSchedule = TRUE;
        }
    }
}

STATIC INLINE BOOL OsSchedScanTimerList(VOID)
{
    UINT32 intSave;
    BOOL needSchedule = FALSE;
    SchedRunqueue *rq = OsGetCurrRunQueue();
    LOS_DL_LIST *listObject;

    SCHED_SL_LOCK(rq, intSave);

    listObject = &(rq->taskSortLinkList->sortLink);
    /*
     * When task is pended with timeout, the task block is on the timeout sortlink
     * (per cpu) and ipc(mutex,sem and etc.)'s block at the same time, it can be waken
     * up by either timeout or corresponding ipc it's waiting.
     *
     * Now synchronize sortlink procedure is used, therefore the whole task scan needs
     * to be protected, preventing another core from doing sortlink deletion at same time.
     */

    if (LOS_ListEmpty(listObject)) {
        SCHED_SL_UNLOCK(rq, intSave);
        return needSchedule;
    }

    SortLinkList *sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
    UINT64 currTime = OsGetCurrSchedTimeCycle();
    while (sortList->responseTime <= currTime) {
        LosTaskCB *taskCB = LOS_DL_LIST_ENTRY(sortList, LosTaskCB, sortList);
        OsDeleteNodeSortLink(&taskCB->sortList);
        OsSchedWakePendTimeTask(taskCB, &needSchedule);
        if (LOS_ListEmpty(listObject)) {
            break;
        }

        sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
    }

    SCHED_SL_UNLOCK(rq, intSave);

    return needSchedule;
}

VOID OsSchedTaskEnQueue(LosTaskCB *taskCB)
{
    UINT32 intSave;
    SchedRunqueue *rq = NULL;

    LOS_ASSERT(!(taskCB->taskStatus & OS_TASK_STATUS_READY));
    if ((taskCB->taskStatus & OS_TASK_STATUS_READY)) return;

    rq = OsGetRunQueue(taskCB->schedRunqueueNum);

    if (!OsIsIdleTask(taskCB->taskID)) {

        SCHED_RQ_LOCK(rq, intSave);

        if (taskCB->timeSlice > g_schedTimeSliceMin) {
            OsSchedPriQueueEnHead(rq, &taskCB->pendList, taskCB->priority);
        } else {
            taskCB->timeSlice = g_schedTimeSlice;
            OsSchedPriQueueEnTail(rq, &taskCB->pendList, taskCB->priority);
        }

        rq->taskNum++;

        SCHED_RQ_UNLOCK(rq, intSave);

        OsHookCall(LOS_HOOK_TYPE_MOVEDTASKTOREADYSTATE, taskCB);
    }

    taskCB->taskStatus &= ~(OS_TASK_STATUS_PEND | OS_TASK_STATUS_SUSPEND |
                            OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME);

    taskCB->taskStatus |= OS_TASK_STATUS_READY;
}

VOID OsSchedTaskDeQueue(LosTaskCB *taskCB)
{
    UINT32 intSave;
    SchedRunqueue *rq = NULL;
    if (NULL == taskCB) return;

    rq = OsGetRunQueue(taskCB->schedRunqueueNum);
    if (taskCB->taskStatus & OS_TASK_STATUS_READY) {
        if (!OsIsIdleTask(taskCB->taskID)) {

            SCHED_RQ_LOCK(rq, intSave);
            OsSchedPriQueueDelete(rq, &taskCB->pendList, taskCB->priority);
            rq->taskNum--;
            SCHED_RQ_UNLOCK(rq, intSave);
        }

        taskCB->taskStatus &= ~OS_TASK_STATUS_READY;
    }
}

VOID OsSchedTaskExit(LosTaskCB *taskCB)
{
    if (taskCB->taskStatus & OS_TASK_STATUS_READY) {
        OsSchedTaskDeQueue(taskCB);
    } else if (taskCB->taskStatus & OS_TASK_STATUS_PEND) {
        LOS_ListDelete(&taskCB->pendList);
        taskCB->taskStatus &= ~OS_TASK_STATUS_PEND;
    }

    if (taskCB->taskStatus & (OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME)) {
        OsDeleteSortLink(&taskCB->sortList);
        taskCB->taskStatus &= ~(OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME);
    }
    taskCB->taskStatus |= OS_TASK_STATUS_EXIT;
}

VOID OsSchedYield(VOID)
{
    LosTask *losTask = OsTaskGet();
    LosTaskCB *runTask = losTask->runTask;

    runTask->timeSlice = 0;
}

VOID OsSchedDelay(LosTaskCB *runTask, UINT32 tick)
{
    runTask->taskStatus |= OS_TASK_STATUS_DELAY;
    runTask->waitTimes = tick;
}

VOID OsSchedTaskWait(LOS_DL_LIST *list, UINT32 ticks)
{
    LosTask *losTask = OsTaskGet();
    LosTaskCB *runTask = losTask->runTask;

    // task status set PEND, means wait for some resource
    runTask->taskStatus |= OS_TASK_STATUS_PEND;

    /* yangzelin@2022-5-23 */
    // clear RUNNING task status
    runTask->taskStatus &= ~OS_TASK_STATUS_RUNNING;

    LOS_ListTailInsert(list, &runTask->pendList);

    if (ticks != LOS_WAIT_FOREVER) {
        runTask->taskStatus |= OS_TASK_STATUS_PEND_TIME;
        runTask->waitTimes = ticks;
    }
}

VOID OsSchedTaskWake(LosTaskCB *resumedTask)
{
    LOS_ListDelete(&resumedTask->pendList);
    resumedTask->taskStatus &= ~OS_TASK_STATUS_PEND;

    if (resumedTask->taskStatus & OS_TASK_STATUS_PEND_TIME) {
        OsDeleteSortLink(&resumedTask->sortList);
        resumedTask->taskStatus &= ~OS_TASK_STATUS_PEND_TIME;
    }

    if (!(resumedTask->taskStatus & OS_TASK_STATUS_SUSPEND) &&
        !(resumedTask->taskStatus & OS_TASK_STATUS_RUNNING)) {
        OsSchedTaskEnQueue(resumedTask);
    }
}

STATIC VOID OsSchedFreezeTask(LosTaskCB *taskCB)
{
    UINT64 responseTime = GET_SORTLIST_VALUE(&taskCB->sortList);
    OsDeleteSortLink(&taskCB->sortList);
    SET_SORTLIST_VALUE(&taskCB->sortList, responseTime);
    taskCB->taskStatus |= OS_TASK_FLAG_FREEZE;
    return;
}

STATIC VOID OsSchedUnfreezeTask(LosTaskCB *taskCB)
{
    UINT32 intSave;
    UINT64 currTime, responseTime;
    UINT32 remainTick;
    SchedRunqueue *rq = NULL;

    taskCB->taskStatus &= ~OS_TASK_FLAG_FREEZE;
    currTime = OsGetCurrSchedTimeCycle();
    responseTime = GET_SORTLIST_VALUE(&taskCB->sortList);

    if (responseTime > currTime) {
        remainTick = ((responseTime - currTime) + OS_CYCLE_PER_TICK - 1) / OS_CYCLE_PER_TICK;
        rq = OsGetRunQueue(taskCB->schedRunqueueNum);

        SCHED_SL_LOCK(rq, intSave);
        OsAdd2SortLink(&taskCB->sortList, currTime, remainTick, rq->taskSortLinkList);
        SCHED_SL_UNLOCK(rq, intSave);
    } 

    SET_SORTLIST_VALUE(&taskCB->sortList, OS_SORT_LINK_INVALID_TIME);
    if (taskCB->taskStatus & OS_TASK_STATUS_PEND) {
        LOS_ListDelete(&taskCB->pendList);
    }
    taskCB->taskStatus &= ~(OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_PEND);
    return;
}

VOID OsSchedSuspend(LosTaskCB *taskCB)
{
    BOOL isPmMode = FALSE;
    if (taskCB->taskStatus & OS_TASK_STATUS_READY) {
        OsSchedTaskDeQueue(taskCB);
    }

#if (LOSCFG_KERNEL_PM == 1)
    isPmMode = OsIsPmMode();
#endif
    if ((taskCB->taskStatus & (OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_DELAY)) && isPmMode) {
        OsSchedFreezeTask(taskCB);
    }

    taskCB->taskStatus |= OS_TASK_STATUS_SUSPEND;
    OsHookCall(LOS_HOOK_TYPE_MOVEDTASKTOSUSPENDEDLIST, taskCB);
}

BOOL OsSchedResume(LosTaskCB *taskCB)
{
    if (taskCB->taskStatus & OS_TASK_FLAG_FREEZE) {
        OsSchedUnfreezeTask(taskCB);
    }

    taskCB->taskStatus &= (~OS_TASK_STATUS_SUSPEND);
    if (!(taskCB->taskStatus & (OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND))) {
        OsSchedTaskEnQueue(taskCB);
        return TRUE;
    }

    return FALSE;
}

BOOL OsSchedModifyTaskSchedParam(LosTaskCB *taskCB, UINT16 priority)
{
    if (taskCB->taskStatus & OS_TASK_STATUS_READY) {
        OsSchedTaskDeQueue(taskCB);
        taskCB->priority = priority;
        OsSchedTaskEnQueue(taskCB);
        return TRUE;
    }

    taskCB->priority = priority;
    OsHookCall(LOS_HOOK_TYPE_TASK_PRIMODIFY, taskCB, taskCB->priority);
    if (taskCB->taskStatus & OS_TASK_STATUS_RUNNING) {
        return TRUE;
    }

    return FALSE;
}

VOID OsSchedTaskReady(LosTaskCB *taskCB)
{
    OsSchedTaskEnQueue(taskCB);
}

VOID OsSchedSetIdleTaskSchedParam(LosTaskCB *idleTask)
{
    OsSchedTaskEnQueue(idleTask);
}

UINT32 OsSchedSwtmrScanRegister(SchedScan func)
{
    if (func == NULL) {
        return LOS_NOK;
    }

    g_swtmrScan = func;
    return LOS_OK;
}

UINT32 OsTaskNextSwitchTimeGet(VOID)
{
    UINT32 intSave;
    SchedRunqueue *rq;
    UINT32 ticks;
    rq = OsGetCurrRunQueue();

    SCHED_SL_LOCK(rq, intSave);
    ticks = OsSortLinkGetNextExpireTime(rq->taskSortLinkList);
    SCHED_SL_UNLOCK(rq, intSave);

    return ticks;
}

UINT64 OsSchedGetNextExpireTime(UINT64 startTime)
{
    return OsGetNextExpireTime(startTime, g_tickResponsePrecision, ArchCurrCpuid());
}

STATIC VOID TaskSchedTimeConvertFreq(UINT32 oldFreq)
{
    for (UINT32 loopNum = 0; loopNum < g_taskMaxNum; loopNum++) {
        LosTaskCB *taskCB = (((LosTaskCB *)g_taskCBArray) + loopNum);
        if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
            continue;
        }
        if (taskCB->timeSlice > 0) {
            taskCB->timeSlice = (INT32)OsTimeConvertFreq((UINT64)taskCB->timeSlice, oldFreq, g_sysClock);
        } else {
            taskCB->timeSlice = 0;
        }

        if (taskCB->taskStatus & OS_TASK_STATUS_RUNNING) {
            taskCB->startTime = OsTimeConvertFreq(taskCB->startTime, oldFreq, g_sysClock);
        }
    }
}

STATIC VOID SchedTimeBaseInit(VOID)
{
    g_schedResponseTime[ArchCurrCpuid()] = OS_SCHED_MAX_RESPONSE_TIME;

    g_schedTickMinPeriod = g_sysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND_MINI;
    g_tickResponsePrecision =  (g_schedTickMinPeriod * 75) / 100; /* 75 / 100: minimum accuracy */
    g_schedTimeSlice = (INT32)(((UINT64)g_sysClock * LOSCFG_BASE_CORE_TIMESLICE_TIMEOUT) / OS_SYS_US_PER_SECOND);
    g_schedTimeSliceMin = (INT32)(((UINT64)g_sysClock * 50) / OS_SYS_US_PER_SECOND); /* Minimum time slice 50 us */
}

VOID OsSchedTimeConvertFreq(UINT32 oldFreq)
{
    SchedTimeBaseInit();
    TaskSchedTimeConvertFreq(oldFreq);
    OsSortLinkResponseTimeConvertFreq(oldFreq);
    OsSchedUpdateExpireTime();
}

STATIC VOID SchedRunqueueInit(SchedRunqueue *rq)
{
    UINT32 intSave;
    UINT16 pri;

    // init spinlock
    LOS_SpinInit(&(rq->priQueueLock));
    LOS_SpinInit(&(rq->sortLinkLock));

    SCHED_RQ_LOCK(rq, intSave);

    for (pri = 0; pri < OS_PRIORITY_QUEUE_NUM; pri++) {
        LOS_ListInit(&(rq->priQueueList[pri]));
    }
    rq->queueBitmap = 0;
    rq->idleTaskID = OS_INVALID;
    rq->swtmrTaskID = OS_INVALID;
    rq->taskNum = 0;

    SCHED_RQ_UNLOCK(rq, intSave);
}

UINT32 OsSchedInit(VOID)
{
    INT32 i;
    SortLinkAttribute *taskSortLinkList;
    SchedRunqueue *rq;

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM;i++) {
        
        // init sort link
        taskSortLinkList = OsGetSortLinkAttribute(OS_SORT_LINK_TASK, i);
        if (taskSortLinkList == NULL) {
            return LOS_NOK;
        }
        OsSortLinkInit(taskSortLinkList);

        // init run queue
        rq = OsGetRunQueue(i);
        SchedRunqueueInit(rq);
        rq->taskSortLinkList = taskSortLinkList;
    }

    SchedTimeBaseInit();

    return LOS_OK;
}

LosTaskCB *OsGetTopTask(VOID)
{
    UINT32 intSave;
    UINT32 priority;
    LosTaskCB *newTask = NULL;

    INT32 cpuid = ArchCurrCpuid();
    SchedRunqueue *rq = OsGetRunQueue(cpuid);

    SCHED_RQ_LOCK(rq, intSave);

    if (rq->queueBitmap) {
        priority = CLZ(rq->queueBitmap);
        newTask = LOS_DL_LIST_ENTRY(((LOS_DL_LIST *)&(rq->priQueueList[priority]))->pstNext, LosTaskCB, pendList);
    } else {
        newTask = OS_TCB_FROM_TID(OsGetCurrIdleTaskID());
    }

    SCHED_RQ_UNLOCK(rq, intSave);

    return newTask;
}

// Pull the task with the highest priority from the ready queue
STATIC LosTaskCB *SchedPickTopTask(VOID)
{
    UINT32 intSave;
    UINT32 priority;
    LosTaskCB *newTask = NULL;

    INT32 cpuid = ArchCurrCpuid();
    SchedRunqueue *rq = OsGetRunQueue(cpuid);

    SCHED_RQ_LOCK(rq, intSave);

    if (rq->queueBitmap != 0) {
        priority = CLZ(rq->queueBitmap);
        newTask = LOS_DL_LIST_ENTRY(((LOS_DL_LIST *)&(rq->priQueueList[priority]))->pstNext, LosTaskCB, pendList);
    } else {
        newTask = OS_TCB_FROM_TID(OsGetIdleTaskID(cpuid));
    }

    if (newTask->taskStatus & OS_TASK_STATUS_READY) {
        if (!OsIsIdleTask(newTask->taskID)) {
            OsSchedPriQueueDelete(rq, &newTask->pendList, newTask->priority);
            rq->taskNum--;
        }

        newTask->taskStatus &= ~OS_TASK_STATUS_READY;
    }
    newTask->cpuid = cpuid & 0xFF;

    SCHED_RQ_UNLOCK(rq, intSave);

    return newTask;
}

VOID OsSchedStart(VOID)
{
    INT32 cpuid = ArchCurrCpuid();
    LosTask *losTask = OsTaskGetByID(cpuid);
    LosTaskCB *newTask = NULL;
#if defined(LOSCFG_KERNEL_SMP)
    PRINTK("core%d Entering scheduler\n", cpuid);
#else
    PRINTK("Entering scheduler\n");
#endif /* LOSCFG_KERNEL_SMP */

    (VOID)LOS_IntLock();

    // step1 : Get the current task with the highest priority
    newTask = SchedPickTopTask();

    // step2 : Task state transition
    newTask->taskStatus |= OS_TASK_STATUS_RUNNING;
    losTask->newTask = newTask;
    losTask->runTask = losTask->newTask;

    // step3 : Timeslice
    newTask->startTime = OsGetCurrSchedTimeCycle();

    // step4 : System start tick
    OsTickSysTimerStartTimeSet(newTask->startTime);

    // step5 : Software timer init
#if (LOSCFG_BASE_CORE_SWTMR == 1)
    OsSwtmrResponseTimeReset(newTask->startTime);
#endif

    // step6 : Mark the scheduling start status
    /* Initialize the schedule timeline and enable scheduling */
    g_taskScheduled = TRUE;
    OS_SCHED_RQ_RUN(cpuid); // set current rq status to SCHED_FLAG_RUN

    // step7 : Scheduling-related variables are initialized
    g_schedResponseTime[cpuid] = OS_SCHED_MAX_RESPONSE_TIME;
    SchedSetCurrResponseID(OS_INVALID);

    // step8 : Update timeslice, reload tick origin value
    OsSchedSetNextExpireTime(newTask->taskID, newTask->startTime + newTask->timeSlice);
}

BOOL OsSchedTaskSwitch(VOID)
{
    UINT32 intSave;
    UINT64 endTime;
    BOOL isTaskSwitch = FALSE;
    LosTaskCB *newTask = NULL;
    INT32 cpuid = ArchCurrCpuid();
    SchedRunqueue *rq = OsGetRunQueue(cpuid);
    LosTask *losTask = OsTaskGetByID(cpuid);
    LosTaskCB *runTask = NULL;

    OsSchedLock(&intSave);

    runTask = losTask->runTask;

    OsTimeSliceUpdate(runTask, OsGetCurrSchedTimeCycle());

    if (OsTaskIslock(cpuid) == TRUE) {
        newTask = losTask->runTask;
    } else {
        if (runTask->waitExit == TRUE) {
            OsRunningTaskDelete(runTask->taskID, runTask);
            runTask->stackPointer = NULL;
            runTask->waitExit = FALSE;
        } else if (runTask->taskStatus & (OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_DELAY)) {
            SCHED_SL_LOCK(rq, intSave);
            OsAdd2SortLink(&runTask->sortList, runTask->startTime, runTask->waitTimes, rq->taskSortLinkList);
            SCHED_SL_UNLOCK(rq, intSave);
        } else if (!(runTask->taskStatus & OS_TASK_BLOCKED_STATUS)) {
            if (!(runTask->taskStatus & OS_TASK_STATUS_READY)) {
                OsSchedTaskEnQueue(runTask);
            }
        }

        newTask = SchedPickTopTask();
    }

    losTask->newTask = newTask;

    if (runTask != newTask) 
    {
#if (LOSCFG_BASE_CORE_TSK_MONITOR == 1)
        OsTaskSwitchCheck();
#endif
        runTask->taskStatus &= ~OS_TASK_STATUS_RUNNING;
        newTask->taskStatus |= OS_TASK_STATUS_RUNNING;

        newTask->startTime = runTask->startTime;

        isTaskSwitch = TRUE;

        OsHookCall(LOS_HOOK_TYPE_TASK_SWITCHEDIN);
    }

    if (!OsIsIdleTask(newTask->taskID)) {
        endTime = newTask->startTime + newTask->timeSlice;
    } else {
        endTime = OS_SCHED_MAX_RESPONSE_TIME - g_tickResponsePrecision;
    }

    if (SchedGetResponseID(cpuid) == runTask->taskID) {
        g_schedResponseTime[cpuid] = OS_SCHED_MAX_RESPONSE_TIME;
    }

    OsSchedSetNextExpireTime(newTask->taskID, endTime);

    OsSchedUnlock(intSave);

    return isTaskSwitch;
}

UINT64 LOS_SchedTickTimeoutNsGet(VOID)
{
    UINT32 intSave;
    UINT64 responseTime;
    UINT64 currTime;

    intSave = LOS_IntLock();
    responseTime = g_schedResponseTime[ArchCurrCpuid()];
    currTime = OsGetCurrSchedTimeCycle();
    LOS_IntRestore(intSave);

    if (responseTime > currTime) {
        responseTime = responseTime - currTime;
    } else {
        responseTime = 0; /* Tick interrupt already timeout */
    }

    return OS_SYS_CYCLE_TO_NS(responseTime, g_sysClock);
}

VOID LOS_SchedTickHandler(VOID)
{
    UINT32 intSave;
    INT32 cpuid = ArchCurrCpuid();
    LosTask *losTask = OsTaskGetByID(cpuid);

    // Check whether the current core is scheduled
    if (!OS_SCHED_RQ_IS_RUNNING(cpuid)) {
        return;
    }

    OsSchedLock(&intSave);

    UINT64 tickStartTime = OsGetCurrSchedTimeCycle();

    if (SchedGetResponseID(cpuid) == OS_INVALID) {

        SCHED_INT_LOCK();

        if (g_swtmrScan != NULL) {
            (VOID)g_swtmrScan();
        }

        (VOID)OsSchedScanTimerList();
        SCHED_INT_UNLOCK();
    }

    OsTimeSliceUpdate(losTask->runTask, tickStartTime);

    losTask->runTask->startTime = OsGetCurrSchedTimeCycle();

    g_schedResponseTime[cpuid] = OS_SCHED_MAX_RESPONSE_TIME;

    if (OsTaskIslock(cpuid) == TRUE) {
        OsSchedUpdateExpireTime();
    } else {
        ArchTaskSchedule();
    }

    OsSchedUnlock(intSave);
}

VOID LOS_Schedule(VOID)
{
    INT32 cpuid = ArchCurrCpuid();

    if (OS_SCHED_RQ_IS_RUNNING(cpuid) && !LOS_TaskIslock(cpuid)) {
        ArchTaskSchedule();
    }
}

VOID OsRunQueueRegisterIdleTask(INT32 cpuid, UINT32 taskid)
{
    g_schedRunqueue[cpuid].idleTaskID = taskid;
}

VOID OsRunQueueRegisterSWTmrTask(INT32 cpuid, UINT32 taskid)
{
    g_schedRunqueue[cpuid].swtmrTaskID = taskid;
}

BOOL OsIsSWTmrTask(UINT32 taskid)
{
    BOOL ret = FALSE;
    SchedRunqueue *rq = NULL;
    int i;

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        rq = OsGetRunQueue(i);

        if (rq->swtmrTaskID == taskid) {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

// select a core by cpuAffiMask
INT32 OsSchedSelectRunQueue(UINT32 cpuAffiMask)
{
    UINT32 intSave;
    INT32 runQueueNum = OS_SCHED_INVALID_RUNQUEUE_NUM;
    UINT16 taskNum = ((UINT16)-1);
    INT32 i;

    OsSchedLock(&intSave);
    // Get the one with the smallest number of tasks among all scheduled queues
    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {

        // can sched on current core
        if (cpuAffiMask & CPUID_TO_AFFI_MASK(i)) {

            if (g_schedRunqueue[i].taskNum <= taskNum) {
                taskNum = g_schedRunqueue[i].taskNum;
                runQueueNum = i;
            }
        }
    }

    OsSchedUnlock(intSave);

    return runQueueNum;
}

/*
    target: target cpu mask
*/
VOID LOS_MpSchedule(UINT32 target)
{
    UINT32 ret;

    UINT32 cpuid = ArchCurrCpuid();
    target &= ~CPUID_TO_AFFI_MASK(cpuid);
    ret = HalIrqSendIpi(target, LOS_MP_IPI_SCHEDULE);
    if (ret != LOS_OK) {
        return;
    }
    return;
}

// If the task status is timeout, cancel the timeout status and return the timeout flag
UINT32 OsSchedTimeoutHandle(LosTaskCB *taskCB)
{
    UINT32 status = 0;

    LOS_ASSERT(taskCB);

    if (taskCB->taskStatus & OS_TASK_STATUS_TIMEOUT) {
        status = OS_TASK_STATUS_TIMEOUT;
        taskCB->taskStatus &= (~OS_TASK_STATUS_TIMEOUT);
    }

    return status;
}

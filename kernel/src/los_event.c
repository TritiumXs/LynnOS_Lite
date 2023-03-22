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

#include "los_event.h"
#include "los_hook.h"
#include "los_interrupt.h"
#include "los_task.h"
#include "los_sched.h"

STATIC VOID EventLock(PEVENT_CB_S eventCB, UINT32 *intSave)
{
    LOS_SpinLockSave(&(eventCB->lock), intSave);
}

STATIC VOID EventUnlock(PEVENT_CB_S eventCB, UINT32 intSave)
{
    LOS_SpinUnlockRestore(&(eventCB->lock), intSave);
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_EventInit(PEVENT_CB_S eventCB)
{
    if (eventCB == NULL) {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }

    LOS_SpinInit(&(eventCB->lock));

    eventCB->uwEventID = 0;
    LOS_ListInit(&eventCB->stEventList);

    OsHookCall(LOS_HOOK_TYPE_EVENT_INIT, eventCB);
    return LOS_OK;
}

LITE_OS_SEC_TEXT UINT32 LOS_EventPoll(UINT32 *eventID, UINT32 eventMask, UINT32 mode)
{
    UINT32 ret = 0;
    UINT32 intSave;

    if (eventID == NULL) {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }
    
    intSave = LOS_IntLock();
    
    if (mode & LOS_WAITMODE_OR) {
        if ((*eventID & eventMask) != 0) {
            ret = *eventID & eventMask;
        }
    } else {
        if ((eventMask != 0) && (eventMask == (*eventID & eventMask))) {
            ret = *eventID & eventMask;
        }
    }
    
    if (ret && (mode & LOS_WAITMODE_CLR)) {
        *eventID = *eventID & ~(ret);
    }
    
    LOS_IntRestore(intSave);
    
    return ret;
}

LITE_OS_SEC_TEXT STATIC_INLINE UINT32 OsEventReadParamCheck(PEVENT_CB_S eventCB, UINT32 eventMask, UINT32 mode)
{
    if (eventCB == NULL) {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }
    
    if ((eventCB->stEventList.pstNext == NULL) || (eventCB->stEventList.pstPrev == NULL)) {
        return LOS_ERRNO_EVENT_NOT_INITIALIZED;
    }
    
    if (eventMask == 0) {
        return LOS_ERRNO_EVENT_EVENTMASK_INVALID;
    }
    
    if (eventMask & LOS_ERRTYPE_ERROR) {
        return LOS_ERRNO_EVENT_SETBIT_INVALID;
    }
    
    if (((mode & LOS_WAITMODE_OR) && (mode & LOS_WAITMODE_AND)) ||
        (mode & ~(LOS_WAITMODE_OR | LOS_WAITMODE_AND | LOS_WAITMODE_CLR)) ||
        !(mode & (LOS_WAITMODE_OR | LOS_WAITMODE_AND))) {
        return LOS_ERRNO_EVENT_FLAGS_INVALID;
    }
    
    return LOS_OK;
}

LITE_OS_SEC_TEXT UINT32 LOS_EventRead(PEVENT_CB_S eventCB, UINT32 eventMask, UINT32 mode, UINT32 timeOut)
{
    UINT32 ret;
    UINT32 intSave, intSave1;
    LosTaskCB *runTsk = NULL;
    LosTask *losTask = OsTaskGet();

    ret = OsEventReadParamCheck(eventCB, eventMask, mode);
    if (ret != LOS_OK) {
        return ret;
    }

    if (OS_INT_ACTIVE) {
        return LOS_ERRNO_EVENT_READ_IN_INTERRUPT;
    }
    
    if (OsTaskIsSystemTask(losTask->runTask)) {
        return LOS_ERRNO_EVENT_READ_IN_SYSTEM_TASK;
    }

    EventLock(eventCB, &intSave);

    ret = LOS_EventPoll(&(eventCB->uwEventID), eventMask, mode);
    OsHookCall(LOS_HOOK_TYPE_EVENT_READ, eventCB, eventMask, mode, timeOut);
    
    if (ret == 0)
    {
        if (timeOut == 0)
        {
            EventUnlock(eventCB, intSave);
            return ret;
        }

        if (OsTaskIslock(ArchCurrCpuid()))
        {
            EventUnlock(eventCB, intSave);
            return LOS_ERRNO_EVENT_READ_IN_LOCK;
        }

        runTsk = losTask->runTask;
        runTsk->eventMask = eventMask;
        runTsk->eventMode = mode;

        // 调度相关
        OsSchedLock(&intSave1);
        OsSchedTaskWait(&eventCB->stEventList, timeOut);
        OsSchedUnlock(intSave1);

        // 解锁
        EventUnlock(eventCB, intSave);

        // 当前任务进入阻塞状态, 触发当前核心的调度
        LOS_Schedule();

        // 再次上锁
        EventLock(eventCB, &intSave);

        // 调度相关, 超时处理
        OsSchedLock(&intSave1);
        if (OsSchedTimeoutHandle(runTsk) == OS_TASK_STATUS_TIMEOUT) {
            runTsk->taskStatus &= ~OS_TASK_STATUS_TIMEOUT;
            OsSchedUnlock(intSave1);
            return LOS_ERRNO_EVENT_READ_TIMEOUT;
        }
        OsSchedUnlock(intSave1);

        ret = LOS_EventPoll(&eventCB->uwEventID, eventMask, mode);
    }

    // 解锁
    EventUnlock(eventCB, intSave);
    return ret;
}

LITE_OS_SEC_TEXT UINT32 LOS_EventWrite(PEVENT_CB_S eventCB, UINT32 events)
{
    LosTaskCB *resumedTask = NULL;
    LosTaskCB *nextTask = (LosTaskCB *)NULL;
    UINT32 intSave, intSave1;
    UINT8 exitFlag = 0;
    
    if (eventCB == NULL) {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }
    
    if ((eventCB->stEventList.pstNext == NULL) || (eventCB->stEventList.pstPrev == NULL)) {
        return LOS_ERRNO_EVENT_NOT_INITIALIZED;
    }
    
    if (events & LOS_ERRTYPE_ERROR) {
        return LOS_ERRNO_EVENT_SETBIT_INVALID;
    }

    EventLock(eventCB, &intSave);

    OsHookCall(LOS_HOOK_TYPE_EVENT_WRITE, eventCB, events);
    
    eventCB->uwEventID |= events;

    if (!LOS_ListEmpty(&eventCB->stEventList))
    {
        for (resumedTask = LOS_DL_LIST_ENTRY((&eventCB->stEventList)->pstNext, LosTaskCB, pendList);
             &resumedTask->pendList != (&eventCB->stEventList);) 
        {
            nextTask = LOS_DL_LIST_ENTRY(resumedTask->pendList.pstNext, LosTaskCB, pendList);

            if (((resumedTask->eventMode & LOS_WAITMODE_OR) && (resumedTask->eventMask & events) != 0) ||
                ((resumedTask->eventMode & LOS_WAITMODE_AND) &&
                 ((resumedTask->eventMask & eventCB->uwEventID) == resumedTask->eventMask)))
            {
                exitFlag = 1;

                // 调度相关
                OsSchedLock(&intSave1);
                OsSchedTaskWake(resumedTask);
                OsSchedUnlock(intSave1);
            }

            resumedTask = nextTask;
        }

        if (exitFlag == 1) {

            EventUnlock(eventCB, intSave);

            // 让所有核心执行一次调度
            LOS_MpSchedule(OS_MP_CPU_ALL);
            LOS_Schedule();
            
            return LOS_OK;
        }
    }

    EventUnlock(eventCB, intSave);

    return LOS_OK;
}

LITE_OS_SEC_TEXT_INIT UINT32 OsEventDestroy(PEVENT_CB_S eventCB)
{
    UINT32 intSave;

    if (eventCB == NULL) {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }

    EventLock(eventCB, &intSave);

    if (!LOS_ListEmpty(&eventCB->stEventList)) {
        EventUnlock(eventCB, intSave);
        return LOS_ERRNO_EVENT_SHOULD_NOT_DESTROYED;
    }

    eventCB->stEventList.pstNext = (LOS_DL_LIST *)NULL;
    eventCB->stEventList.pstPrev = (LOS_DL_LIST *)NULL;

    EventUnlock(eventCB, intSave);

    return LOS_OK;
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_EventDestroy(PEVENT_CB_S eventCB)
{
    UINT32 ret;
    UINT32 intSave;

    OsSchedLock(&intSave);
    ret = OsEventDestroy(eventCB);
    OsSchedUnlock(intSave);

    if (LOS_OK == ret) {
        OsHookCall(LOS_HOOK_TYPE_EVENT_DESTROY, eventCB);
    }

    return ret;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_EventClear(PEVENT_CB_S eventCB, UINT32 eventMask)
{
    UINT32 intSave;
    
    if (eventCB == NULL) {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }
    
    OsHookCall(LOS_HOOK_TYPE_EVENT_CLEAR, eventCB, eventMask);
    
    EventLock(eventCB, &intSave);
    eventCB->uwEventID &= eventMask;
    EventUnlock(eventCB, intSave);

    return LOS_OK;
}

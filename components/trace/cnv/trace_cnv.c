/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: LiteOS Trace Cnv Implementation
 * Author: Huawei LiteOS Team
 * Create: 2020-07-01
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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
 * --------------------------------------------------------------------------- */

#include "trace_cnv.h"
#include "los_trace.h"
#include "los_task.h"
#include "los_sem.h"
#include "los_mux.h"
#include "los_queue.h"
#include "los_event.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

VOID OsTraceCnvHookData(UINT32 type, ...) 
{
	va_list ap;
	va_start(ap, type);

	switch(type) {
		case MEM_ALLOC: {
			VOID *pool = va_arg(ap, VOID *);
			VOID *ptr = va_arg(ap, VOID *);
			UINT32 size = va_arg(ap, UINT32);
			LOS_TRACE(MEM_ALLOC, pool, (UINTPTR)ptr, size);
			break;		
		}
		
		case MEM_FREE: {
			VOID *pool = va_arg(ap, VOID *);
			VOID *ptr = va_arg(ap, VOID *);
			LOS_TRACE(MEM_FREE, pool, (UINTPTR)ptr);
			break;
		}
		
		case MEM_REALLOC: {
			VOID *pool = va_arg(ap, VOID *);
			VOID *ptr = va_arg(ap, VOID *);
			UINT32 size = va_arg(ap, UINT32);
			LOS_TRACE(MEM_REALLOC, pool, (UINTPTR)ptr, size);
			break;
		}

		case MEM_ALLOC_ALIGN: {
			VOID *pool = va_arg(ap, VOID *);
			VOID *ptr = va_arg(ap, VOID *);
			UINT32 size = va_arg(ap, UINT32);
			UINT32 boundary = va_arg(ap, UINT32);
			LOS_TRACE(MEM_ALLOC_ALIGN, pool, (UINTPTR)ptr, size, boundary);
			break;
		}

		case MEM_INFO_REQ: {
			VOID *pool = va_arg(ap, VOID *);
			LOS_TRACE(MEM_INFO_REQ, pool);
			break;
		}

		case EVENT_CREATE: {
			PEVENT_CB_S eventCB = va_arg(ap, PEVENT_CB_S);
			LOS_TRACE(EVENT_CREATE, (UINTPTR)eventCB);
			break;			
		}

		case EVENT_DELETE: {
			PEVENT_CB_S eventCB = va_arg(ap, PEVENT_CB_S);
			LOS_TRACE(EVENT_DELETE, (UINTPTR)eventCB, LOS_OK);
			break;
		}

		case EVENT_READ: {
			PEVENT_CB_S eventCB = va_arg(ap, PEVENT_CB_S);
			UINT32 eventMask = va_arg(ap, UINT32);
			UINT32 mode = va_arg(ap, UINT32);
			UINT32 timeout = va_arg(ap, UINT32);
			LOS_TRACE(EVENT_READ, (UINTPTR)eventCB, eventCB->uwEventID, eventMask, mode, timeout);
			break;
		}

		case EVENT_WRITE: {
			PEVENT_CB_S eventCB = va_arg(ap, PEVENT_CB_S);
			UINT32 events = va_arg(ap, UINT32);
			LOS_TRACE(EVENT_WRITE, (UINTPTR)eventCB, eventCB->uwEventID, events);
			break;
		}

		case EVENT_CLEAR: {
			PEVENT_CB_S eventCB = va_arg(ap, PEVENT_CB_S);
			UINT32 events = va_arg(ap, UINT32);
			LOS_TRACE(EVENT_CLEAR, (UINTPTR)eventCB, eventCB->uwEventID, events);
			break;
		}

		case QUEUE_CREATE: {
			LosQueueCB *queueCB = va_arg(ap, LosQueueCB *);
			LOS_TRACE(QUEUE_CREATE, queueCB->queueID, queueCB->queueLen, queueCB->queueSize - sizeof(UINT32), (UINTPTR)queueCB, 0);
			break;
		}

		case QUEUE_DELETE: {
			LosQueueCB *queueCB = va_arg(ap, LosQueueCB *);
			LOS_TRACE(QUEUE_DELETE, queueCB->queueID, queueCB->queueState, queueCB->readWriteableCnt[OS_QUEUE_READ]);
			break;
		}

		case QUEUE_RW: {
			LosQueueCB *queueCB = va_arg(ap, LosQueueCB *);
			UINT32 operateType = va_arg(ap, UINT32);
			UINT32 bufferSize = va_arg(ap, UINT32);
			UINT32 timeout = va_arg(ap, UINT32);
			LOS_TRACE(QUEUE_RW, queueCB->queueID, queueCB->queueSize, bufferSize, operateType, queueCB->readWriteableCnt[OS_QUEUE_READ],
					queueCB->readWriteableCnt[OS_QUEUE_WRITE], timeout);
			break;
		}

		case SEM_CREATE: {
			LosSemCB *semCB = va_arg(ap, LosSemCB *);
			LOS_TRACE(SEM_CREATE, semCB->semID, 0, semCB->semCount);
			break;
		}

		case SEM_DELETE: {
			LosSemCB *semCB = va_arg(ap, LosSemCB *);
			LOS_TRACE(SEM_DELETE, semCB->semID, LOS_OK);
			break;
		}

		case SEM_PEND: {
			LosSemCB *semCB = va_arg(ap, LosSemCB *);
			LosTaskCB *runningTask = va_arg(ap, LosTaskCB *);
			UINT32 timeout = va_arg(ap, UINT32);
			(VOID)runningTask;
			LOS_TRACE(SEM_PEND, semCB->semID, semCB->semCount, timeout);
			break;
		}

		case SEM_POST: {
			LosSemCB *semCB = va_arg(ap, LosSemCB *);
			LosTaskCB *resumedTask = va_arg(ap, LosTaskCB *);
			(VOID)resumedTask;
			LOS_TRACE(SEM_POST, semCB->semID, 0, semCB->semCount);
			break;
		}

		case MUX_CREATE: {
			LosMuxCB *muxCB = va_arg(ap, LosMuxCB *);
			LOS_TRACE(MUX_CREATE, muxCB->muxID);
			break;
		}

		case MUX_DELETE: {
			LosMuxCB *muxCB = va_arg(ap, LosMuxCB *);
			LOS_TRACE(MUX_DELETE, muxCB->muxID, muxCB->muxStat, muxCB->muxCount, (muxCB->owner == NULL) ? 0xffffffff : muxCB->owner->taskID);
			break;
		}

		case MUX_PEND: {
			LosMuxCB *muxCB = va_arg(ap, LosMuxCB *);
			UINT32 timeout = va_arg(ap, UINT32);
			LOS_TRACE(MUX_PEND, muxCB->muxID, muxCB->muxCount, (muxCB->owner == NULL) ? 0xffffffff : muxCB->owner->taskID, timeout);
			break;
		}
	
		case MUX_POST: {
			LosMuxCB *muxCB = va_arg(ap, LosMuxCB *);
			LOS_TRACE(MUX_POST, muxCB->muxID, muxCB->muxCount, (muxCB->owner == NULL) ? 0xffffffff : muxCB->owner->taskID);
			break;
		}

		case TASK_CREATE: {
			LosTaskCB *taskCB = va_arg(ap, LosTaskCB *);
			LOS_TRACE(TASK_CREATE, taskCB->taskID, taskCB->taskStatus, taskCB->priority);
			break;			
		}

		case TASK_PRIOSET: {
			LosTaskCB *taskCB = va_arg(ap, LosTaskCB *);
			UINT32 prio = va_arg(ap, UINT32);
			LOS_TRACE(TASK_PRIOSET, taskCB->taskID, taskCB->taskStatus, taskCB->priority, prio);
			break;
		}

		case TASK_SUSPEND: {
			LosTaskCB *taskCB = va_arg(ap, LosTaskCB *);
			LOS_TRACE(TASK_SUSPEND, taskCB->taskID, taskCB->taskStatus, g_losTask.runTask->taskID);
			break;
		}

		case TASK_RESUME: {
			LosTaskCB *taskCB = va_arg(ap, LosTaskCB *);
			LOS_TRACE(TASK_RESUME, taskCB->taskID, taskCB->taskStatus, taskCB->priority);
			break;
		}

		case TASK_DELETE: {
			LosTaskCB *taskCB = va_arg(ap, LosTaskCB *);
			LOS_TRACE(TASK_DELETE, taskCB->taskID, taskCB->taskStatus, (UINTPTR)taskCB->stackPointer);
			break;
		}

		case TASK_SWITCH: {
			LosTaskCB *newTask = g_losTask.newTask;
			LosTaskCB *runTask = g_losTask.runTask;
			LOS_TRACE(TASK_SWITCH, newTask->taskID, runTask->priority, runTask->taskStatus, newTask->priority, newTask->taskStatus);
			break;
		}

		case HWI_RESPONSE_IN: {
			UINT32 hwiNum = va_arg(ap, UINT32);
			LOS_TRACE(HWI_RESPONSE_IN, hwiNum);
			break;
		}

		case HWI_RESPONSE_OUT: {
			UINT32 hwiNum = va_arg(ap, UINT32);
			LOS_TRACE(HWI_RESPONSE_OUT, hwiNum);
			break;
		}

		default: {
			break;
		}
	}
	va_end(ap);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

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

#ifndef _TRACE_CNV_H
#define _TRACE_CNV_H

#include "los_trace.h"
#include "los_task.h"

#include "stdarg.h"
#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

extern VOID OsTraceCnvHookData(UINT32 type, ...);

#if (LOSCFG_KERNEL_TRACE == 1)
#define LOS_TRACE_CNV(hookType, args...) \
	do { \
		UINT32 _type; \
		switch (hookType) { \
			case LOS_HOOK_TYPE_MEM_ALLOC: \
				_type = MEM_ALLOC; \
				break;\
			case LOS_HOOK_TYPE_MEM_FREE: \
				_type = MEM_FREE; \
				break; \
			case LOS_HOOK_TYPE_MEM_INIT: \
				_type = MEM_INFO_REQ; \
				break;\
			case LOS_HOOK_TYPE_MEM_REALLOC: \
				_type = MEM_REALLOC; \
				break; \
			case LOS_HOOK_TYPE_MEM_ALLOCALIGN: \
				_type = MEM_ALLOC_ALIGN; \
				break;\
			case LOS_HOOK_TYPE_EVENT_INIT: \
				_type = EVENT_CREATE; \
				break; \
			case LOS_HOOK_TYPE_EVENT_READ: \
				_type = EVENT_READ; \
				break;\
			case LOS_HOOK_TYPE_EVENT_WRITE: \
				_type = EVENT_WRITE; \
				break; \
			case LOS_HOOK_TYPE_EVENT_CLEAR: \
				_type = EVENT_CLEAR; \
				break;\
			case LOS_HOOK_TYPE_EVENT_DESTROY: \
				_type = EVENT_DELETE; \
				break; \
			case LOS_HOOK_TYPE_QUEUE_CREATE: \
				_type = QUEUE_CREATE; \
				break;\
			case LOS_HOOK_TYPE_QUEUE_DELETE: \
				_type = QUEUE_DELETE; \
				break; \
			case LOS_HOOK_TYPE_QUEUE_READ: \
				_type = QUEUE_RW; \
				break;\
			case LOS_HOOK_TYPE_QUEUE_WRITE: \
				_type = QUEUE_RW; \
				break; \
			case LOS_HOOK_TYPE_SEM_CREATE: \
				_type = SEM_CREATE; \
				break;\
			case LOS_HOOK_TYPE_SEM_DELETE: \
				_type = SEM_DELETE; \
				break; \
			case LOS_HOOK_TYPE_SEM_POST: \
				_type = SEM_POST; \
				break;\
			case LOS_HOOK_TYPE_SEM_PEND: \
				_type = SEM_PEND; \
				break; \
			case LOS_HOOK_TYPE_MUX_CREATE: \
				_type = MUX_CREATE; \
				break;\
			case LOS_HOOK_TYPE_MUX_POST: \
				_type = MUX_POST; \
				break; \
			case LOS_HOOK_TYPE_MUX_PEND: \
				_type = MUX_PEND; \
				break;\
			case LOS_HOOK_TYPE_MUX_DELETE: \
				_type = MUX_DELETE; \
				break; \
			case LOS_HOOK_TYPE_TASK_PRIMODIFY: \
				_type = TASK_PRIOSET; \
				break;\
			case LOS_HOOK_TYPE_TASK_DELETE: \
				_type = TASK_DELETE; \
				break; \
			case LOS_HOOK_TYPE_TASK_CREATE: \
				_type = TASK_CREATE; \
				break;\
			case LOS_HOOK_TYPE_TASK_SWITCHEDIN: \
				_type = TASK_SWITCH; \
				break; \
			case LOS_HOOK_TYPE_MOVEDTASKTOREADYSTATE: \
				_type = TASK_RESUME; \
				break;\
			case LOS_HOOK_TYPE_MOVEDTASKTOSUSPENDEDLIST: \
				_type = TASK_SUSPEND; \
				break; \
			case LOS_HOOK_TYPE_ISR_ENTER: \
				_type = HWI_RESPONSE_IN; \
				break;\
			case LOS_HOOK_TYPE_ISR_EXIT: \
				_type = HWI_RESPONSE_OUT; \
				break; \
			default: \
				 _type = 0; \
				break; \
		} \
		OsTraceCnvHookData(_type, ##args); \
	} while (0)
#else
#define LOS_TRACE_CNV(hookType, args...)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _TRACE_CNV_H */

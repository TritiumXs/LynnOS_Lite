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

#include "los_context.h"
#include "los_arch_context.h"
#include "los_arch_timer.h"
#include "los_timer.h"
#include "los_task.h"
#include "csfr.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SOFT_PENDING      (1UL << 3)

LITE_OS_SEC_TEXT_INIT VOID ArchInit(VOID)
{
    ArchHwiInit();
}

LITE_OS_SEC_TEXT_MINOR VOID ArchSysExit(VOID)
{
    ArchIntLock();
    while (1) {
    }
}

LITE_OS_SEC_TEXT_INIT VOID *ArchTskStackInit(UINT32 taskID, UINT32 stackSize, VOID *topStack)
{
    TaskContext  *context = NULL;

    printf("    task stack init %x  size:%d\n", topStack, stackSize);
    /* initialize the task stack, write magic num to stack top */
    for (UINT32 index = 1; index < (stackSize / sizeof(UINT32)); index++) {
        *((UINT32 *)topStack + index) = OS_TASK_STACK_INIT;
    }
    *((UINT32 *)(topStack)) = OS_TASK_MAGIC_WORD;

    context = (TaskContext *)(((UINTPTR)topStack + stackSize) - sizeof(TaskContext));

    context->psr = 0;
    context->rets = (UINT32)ArchSysExit;
    context->reti = (UINT32)OsTaskEntry;
    context->r0 = (UINT32)taskID;

    return (VOID *)context;
}

LITE_OS_SEC_TEXT_INIT VOID *ArchSignalContextInit(VOID *stackPointer, VOID *stackTop, UINTPTR sigHandler, UINT32 param)
{
    UNUSED(stackTop);
    TaskContext *context = (TaskContext *)((UINTPTR)stackPointer - sizeof(TaskContext));
    (VOID)memset_s((VOID *)context, sizeof(TaskContext), 0, sizeof(TaskContext));

    context->r0 = param;
    context->rets = (UINT32)ArchSysExit;
    context->reti = sigHandler;
    context->psr = 0x0; /* Thumb flag, always set 1 */

    return (VOID *)context;
}

LITE_OS_SEC_TEXT_INIT UINT32 ArchStartSchedule(VOID)
{
    (VOID)LOS_IntLock();
    OsSchedStart();

    printf("    start to run: %s, stack %x\n", g_losTask.runTask->taskName,
           g_losTask.runTask->stackPointer);
    printf("    new task %x, %s, stack %x\n",
           g_losTask.newTask, g_losTask.newTask->taskName,
           g_losTask.newTask->stackPointer);

    ArchStartToRun();
    return LOS_OK; /* never return */
}

VOID ArchTaskSchedule(VOID)
{
    // clear watch dog
    wdt_clear();
    int cpu = 0; // cpu 0
    q32DSP(cpu)->ILAT_SET |= SOFT_PENDING;
}

VOID ArchClearSchedulePending(VOID)
{
    int cpu = 0;
    q32DSP(cpu)->ILAT_CLR = SOFT_PENDING;
}

BOOL ArchTaskSwitch(VOID)
{
    BOOL isSwitch = OsSchedTaskSwitch();
    if (isSwitch) {
        g_losTask.runTask = g_losTask.newTask;
        return TRUE;
    } else {
        return FALSE;
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

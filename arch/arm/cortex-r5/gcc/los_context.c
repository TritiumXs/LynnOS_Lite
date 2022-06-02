/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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
#include "securec.h"
#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_interrupt.h"
#include "los_debug.h"

/* ****************************************************************************
 Function    : ArchInit
 Description : arch init function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT VOID ArchInit(VOID)
{
    HalHwiInit();
}

/* ****************************************************************************
 Function    : ArchSysExit
 Description : Task exit function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT_MINOR VOID ArchSysExit(VOID)
{
    LOS_IntLock();
    while (1) {
    }
}

/* ****************************************************************************
 Function    : ArchTskStackInit
 Description : Task stack initialization function
 Input       : taskID     --- TaskID
               stackSize  --- Total size of the stack
               topStack    --- Top of task's stack
 Output      : None
 Return      : Context pointer
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT VOID *ArchTskStackInit(UINT32 taskID, UINT32 stackSize, VOID *topStack)
{
    TaskContext *context = (TaskContext *)((UINTPTR)topStack + stackSize - sizeof(TaskContext));
    LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);
    UINT32 cpsr;

    context->r0 = taskID;
    context->r1 = 0x01010101L;
    context->r2 = 0x02020202L;
    context->r3 = 0x03030303L;
    context->r4 = 0x04040404L;
    context->r5 = 0x05050505L;
    context->r6 = 0x06060606L;
    context->r7 = 0x07070707L;
    context->r8 = 0x08080808L;
    context->r9 = 0x09090909L;
    context->r10 = 0x10101010L;
    context->r11 = 0x11111111L;
    context->r12 = 0x12121212L;
    context->sp = (UINTPTR)topStack + stackSize;
    context->lr = (UINTPTR)ArchSysExit;
    cpsr = HalGetCpsr();
    if ((UINTPTR)taskCB->taskEntry & 0x01) {
        context->pc = (UINTPTR)OsTaskEntryThumb;
        context->spsr = PSR_MODE_SYS_THUMB | (cpsr & PSR_E_MASK); /* thumb mode */
    } else {
        context->pc = (UINTPTR)OsTaskEntryArm;
        context->spsr = PSR_MODE_SYS_ARM | (cpsr & PSR_E_MASK);   /* arm mode */
    }

    return (VOID *)context;
}

LITE_OS_SEC_TEXT_INIT UINT32 ArchStartSchedule(VOID)
{
    (VOID)LOS_IntLock();
    OsSchedStart();
    HalStartToRun();

    return LOS_OK; /* never return */
}

BOOL g_ScheduleInIrq = FALSE;
LITE_OS_SEC_TEXT_INIT VOID ArchTaskSchedule(VOID)
{
    UINT32 cpsr;
    cpsr = HalGetCpsr();
    if ((cpsr & PSR_MODE_MASK) == PSR_MODE_SYS) {
        __asm__ __volatile__(" swi #0");
    } else {
        g_ScheduleInIrq = TRUE;
    }
}

LITE_OS_SEC_TEXT_INIT VOID dmb(VOID)
{
    __asm __volatile(" dmb");
}

LITE_OS_SEC_TEXT_INIT VOID dsb(VOID)
{
    __asm __volatile(" dsb");
}

LITE_OS_SEC_TEXT_INIT VOID isb(VOID)
{
    __asm __volatile(" isb");
}

LITE_OS_SEC_TEXT_INIT VOID wfi(VOID)
{
    __asm __volatile(" wfi");
}


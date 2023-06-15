/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include <stdarg.h>
#include "securec.h"
#include "los_context.h"
#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_memory.h"
#include "los_membox.h"

#define INT_OFFSET       6
#define PRI_OFF_PER_INT  8
#define PRI_PER_REG      4
#define PRI_OFF_IN_REG   6
#define PRI_BITS         2
#define PRI_HI           0
#define PRI_LOW          7
#define MASK_8_BITS      0xFF
#define MASK_32_BITS     0xFFFFFFFF
#define BYTES_OF_128_INT 4

CHAR g_trapStackBase[OS_TRAP_STACK_SIZE];

VIC_TYPE *VIC_REG = (VIC_TYPE *)VIC_REG_BASE;

UINT32 HalGetPsr(VOID)
{
    UINT32 intSave;
    __asm__ volatile("mfcr %0, psr" : "=r" (intSave) : : "memory");
    return intSave;
}

UINT32 HalSetVbr(UINT32 intSave)
{
    __asm__ volatile("mtcr %0, vbr" : : "r"(intSave)  : "memory");
    return intSave;
}

UINT32 ArchIntLock(VOID)
{
    UINT32 intSave;
    __asm__ __volatile__(
        "mfcr    %0, psr \n"
        "psrclr ie"
        : "=r"(intSave)
        :
        : "memory");
    return intSave;
}

UINT32 ArchIntUnLock(VOID)
{
    UINT32 intSave;
    __asm__ __volatile__(
        "mfcr   %0, psr \n"
        "psrset ie"
        : "=r"(intSave)
        :
        : "memory");
    return intSave;
}

VOID ArchIntRestore(UINT32 intSave)
{
    __asm__ __volatile__("mtcr %0, psr" : : "r"(intSave));
}

UINT32 ArchIntLocked(VOID)
{
    UINT32 intSave;
    __asm__ volatile("mfcr %0, psr" : "=r" (intSave) : : "memory");
    return !(intSave & (1 << INT_OFFSET));
}

STATIC UINT32 HwiUnmask(HWI_HANDLE_T hwiNum)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    VIC_REG->ISER[hwiNum / OS_SYS_VECTOR_CNT] = (UINT32)(1UL << (hwiNum % OS_SYS_VECTOR_CNT));
    VIC_REG->ISSR[hwiNum / OS_SYS_VECTOR_CNT] = (UINT32)(1UL << (hwiNum % OS_SYS_VECTOR_CNT));
    LOS_IntRestore(intSave);

    return LOS_OK;
}

STATIC UINT32 HwiSetPriority(HWI_HANDLE_T hwiNum, UINT8 priority)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    VIC_REG->IPR[hwiNum / PRI_PER_REG] |= (((priority << PRI_OFF_IN_REG) << (hwiNum % PRI_PER_REG)) * PRI_OFF_PER_INT);
    LOS_IntRestore(intSave);

    return LOS_OK;
}

STATIC UINT32 HwiMask(HWI_HANDLE_T hwiNum)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    VIC_REG->ICER[hwiNum / OS_SYS_VECTOR_CNT] = (UINT32)(1UL << (hwiNum % OS_SYS_VECTOR_CNT));
    LOS_IntRestore(intSave);

    return LOS_OK;
}

STATIC UINT32 HwiPending(HWI_HANDLE_T hwiNum)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    VIC_REG->ISPR[hwiNum / OS_SYS_VECTOR_CNT] = (UINT32)(1UL << (hwiNum % OS_SYS_VECTOR_CNT));
    LOS_IntRestore(intSave);

    return LOS_OK;
}

STATIC UINT32 HwiClear(HWI_HANDLE_T hwiNum)
{
    VIC_REG->ICPR[hwiNum / OS_SYS_VECTOR_CNT] = (UINT32)(1UL << (hwiNum % OS_SYS_VECTOR_CNT));

    return LOS_OK;
}

/* ****************************************************************************
 Function    : HwiNumGet
 Description : Get an interrupt number
 Input       : None
 Output      : None
 Return      : Interrupt Indexes number
 **************************************************************************** */
STATIC UINT32 HwiNumGet(VOID)
{
    return (HalGetPsr() >> PSR_VEC_OFFSET) & MASK_8_BITS;
}

STATIC UINT32 HwiCreate(HWI_HANDLE_T hwiNum, HWI_PRIOR_T hwiPrio)
{
    HwiSetPriority(hwiNum, (UINT8)hwiPrio);
    HwiUnmask(hwiNum);
    return LOS_OK;
}

STATIC HwiControllerOps g_archHwiOps = {
    .enableIrq      = HwiUnmask,
    .disableIrq     = HwiMask,
    .setIrqPriority = HwiSetPriority,
    .getCurIrqNum   = HwiNumGet,
    .triggerIrq     = HwiPending,
    .clearIrq       = HwiClear,
    .createIrq      = HwiCreate,
};

HwiControllerOps *ArchIntOpsGet(VOID)
{
    return &g_archHwiOps;
}

/* ****************************************************************************
 Function    : HalInterrupt
 Description : Hardware interrupt entry function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT VOID HalInterrupt(VOID)
{
    UINT32 hwiIndex;
    UINT32 intSave;

    intSave = LOS_IntLock();
    g_intCount++;
    LOS_IntRestore(intSave);

    hwiIndex = HwiNumGet();
    OsHookCall(LOS_HOOK_TYPE_ISR_ENTER, hwiIndex);

    HalPreInterruptHandler(hwiIndex);
#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
    if (g_hwiHandlerForm[hwiIndex].pfnHandler != 0) {
        g_hwiHandlerForm[hwiIndex].pfnHandler((VOID *)g_hwiHandlerForm[hwiIndex].pParm);
    }
#else
    if (g_hwiHandlerForm[hwiIndex] != 0) {
        g_hwiHandlerForm[hwiIndex]();
    }
#endif

    HalAftInterruptHandler(hwiIndex);

    OsHookCall(LOS_HOOK_TYPE_ISR_EXIT, hwiIndex);

    intSave = LOS_IntLock();
    g_intCount--;
#ifndef CPU_CK804
    HalIrqEndCheckNeedSched();
#endif
    LOS_IntRestore(intSave);
}

ExcInfo g_excInfo = {0};

#if (LOSCFG_KERNEL_PRINTF != 0)
STATIC VOID OsExcTypeInfo(const ExcInfo *excInfo)
{
    CHAR *phaseStr[] = {"exc in init", "exc in task", "exc in hwi"};

    PRINTK("Type      = %d\n", excInfo->type);
    PRINTK("ThrdPid   = %d\n", excInfo->thrdPid);
    PRINTK("Phase     = %s\n", phaseStr[excInfo->phase]);
    PRINTK("FaultAddr = 0x%x\n", excInfo->faultAddr);
}

STATIC VOID OsExcCurTaskInfo(const ExcInfo *excInfo)
{
    PRINTK("Current task info:\n");
    if (excInfo->phase == OS_EXC_IN_TASK) {
        LosTaskCB *taskCB = OS_TCB_FROM_TID(LOS_CurTaskIDGet());
        PRINTK("Task name = %s\n", taskCB->taskName);
        PRINTK("Task ID   = %d\n", taskCB->taskID);
        PRINTK("Task SP   = 0x%x\n", (UINTPTR)taskCB->stackPointer);
        PRINTK("Task ST   = 0x%x\n", taskCB->topOfStack);
        PRINTK("Task SS   = 0x%x\n", taskCB->stackSize);
    } else if (excInfo->phase == OS_EXC_IN_HWI) {
        PRINTK("Exception occur in interrupt phase!\n");
    } else {
        PRINTK("Exception occur in system init phase!\n");
    }
}

STATIC VOID OsExcRegInfo(const ExcInfo *excInfo)
{
    EXC_CONTEXT_S *excContext = excInfo->context;
    PRINTK("Exception reg dump:\n");
    PRINTK("R0         = 0x%x\n"
           "R1         = 0x%x\n"
           "R2         = 0x%x\n"
           "R3         = 0x%x\n"
           "R4         = 0x%x\n"
           "R5         = 0x%x\n"
           "R6         = 0x%x\n"
           "R7         = 0x%x\n"
           "R8         = 0x%x\n"
           "R9         = 0x%x\n"
           "R10        = 0x%x\n"
           "R11        = 0x%x\n"
           "R12        = 0x%x\n"
           "R13        = 0x%x\n"
           "R14        = 0x%x\n"
           "R15        = 0x%x\n"
           "EPSR       = 0x%x\n"
           "EPC        = 0x%x\n",
           excContext->R0, excContext->R1, excContext->R2, excContext->R3, excContext->R4, excContext->R5,
           excContext->R6, excContext->R7, excContext->R8, excContext->R9, excContext->R10, excContext->R11,
           excContext->R12, excContext->R13, excContext->R14, excContext->R15, excContext->EPSR,
           excContext->EPC);
}

#if (LOSCFG_KERNEL_BACKTRACE == 1)
STATIC VOID OsExcBackTraceInfo(const ExcInfo *excInfo)
{
    UINTPTR LR[LOSCFG_BACKTRACE_DEPTH] = {0};
    UINT32 index;

    OsBackTraceHookCall(LR, LOSCFG_BACKTRACE_DEPTH, 0, excInfo->context->R14);

    PRINTK("----- backtrace start -----\n");
    for (index = 0; index < LOSCFG_BACKTRACE_DEPTH; index++) {
        if (LR[index] == 0) {
            break;
        }
        PRINTK("backtrace %d -- lr = 0x%x\n", index, LR[index]);
    }
    PRINTK("----- backtrace end -----\n");
}
#endif

STATIC VOID OsExcMemPoolCheckInfo(VOID)
{
    PRINTK("\r\nmemory pools check:\n");
#if (LOSCFG_PLATFORM_EXC == 1)
    MemInfoCB memExcInfo[OS_SYS_MEM_NUM];
    UINT32 errCnt;
    UINT32 i;

    (VOID)memset_s(memExcInfo, sizeof(memExcInfo), 0, sizeof(memExcInfo));

    errCnt = OsMemExcInfoGet(OS_SYS_MEM_NUM, memExcInfo);
    if (errCnt < OS_SYS_MEM_NUM) {
        errCnt += OsMemboxExcInfoGet(OS_SYS_MEM_NUM - errCnt, memExcInfo + errCnt);
    }

    if (errCnt == 0) {
        PRINTK("all memory pool check passed!\n");
        return;
    }

    for (i = 0; i < errCnt; i++) {
        PRINTK("pool num    = %d\n", i);
        PRINTK("pool type   = %d\n", memExcInfo[i].type);
        PRINTK("pool addr   = 0x%x\n", memExcInfo[i].startAddr);
        PRINTK("pool size   = 0x%x\n", memExcInfo[i].size);
        PRINTK("pool free   = 0x%x\n", memExcInfo[i].free);
        PRINTK("pool blkNum = %d\n", memExcInfo[i].blockSize);
        PRINTK("pool error node addr  = 0x%x\n", memExcInfo[i].errorAddr);
        PRINTK("pool error node len   = 0x%x\n", memExcInfo[i].errorLen);
        PRINTK("pool error node owner = %d\n", memExcInfo[i].errorOwner);
    }
#endif
    UINT32 ret = LOS_MemIntegrityCheck(LOSCFG_SYS_HEAP_ADDR);
    if (ret == LOS_OK) {
        PRINTK("system heap memcheck over, all passed!\n");
    }

    PRINTK("memory pool check end!\n");
}
#endif

STATIC VOID OsExcInfoDisplay(const ExcInfo *excInfo)
{
#if (LOSCFG_KERNEL_PRINTF != 0)
    PRINTK("*************Exception Information**************\n");
    OsExcTypeInfo(excInfo);
    OsExcCurTaskInfo(excInfo);
    OsExcRegInfo(excInfo);
#if (LOSCFG_KERNEL_BACKTRACE == 1)
    OsExcBackTraceInfo(excInfo);
#endif
    OsGetAllTskInfo();
    OsExcMemPoolCheckInfo();
#endif
}

LITE_OS_SEC_TEXT_INIT VOID HalExcHandleEntry(EXC_CONTEXT_S *excBufAddr, UINT32 faultAddr)
{
    UINT16 tmpFlag = ((excBufAddr->EPSR >> PSR_VEC_OFFSET) & MASK_8_BITS);
    g_excInfo.nestCnt++;
    UINT32 excType = (HalGetPsr() >> PSR_VEC_OFFSET) & MASK_8_BITS;
    g_excInfo.type = excType;

    g_excInfo.faultAddr = faultAddr;

    if (g_losTask.runTask != NULL) {
        if (tmpFlag > 0) {
            g_excInfo.phase = OS_EXC_IN_HWI;
            g_excInfo.thrdPid = tmpFlag;
        } else {
            g_excInfo.phase = OS_EXC_IN_TASK;
            g_excInfo.thrdPid = g_losTask.runTask->taskID;
        }
    } else {
        g_excInfo.phase = OS_EXC_IN_INIT;
        g_excInfo.thrdPid = OS_NULL_INT;
    }
    g_excInfo.context = excBufAddr;

    OsDoExcHook(EXC_INTERRUPT);
    OsExcInfoDisplay(&g_excInfo);
    ArchSysExit();
}

/* stack protector */
WEAK UINT32 __stack_chk_guard = 0xd00a0dff;

WEAK VOID __stack_chk_fail(VOID)
{
    /* __builtin_return_address is a builtin function, building in gcc */
    LOS_Panic("stack-protector: Kernel stack is corrupted in: %x\n",
              __builtin_return_address(0));
}

WEAK void HalHwiHandleReInit(UINT32 hwiFormAddr)
{
}

/* ****************************************************************************
 Function    : HalHwiInit
 Description : initialization of the hardware interrupt
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT VOID HalHwiInit(VOID)
{
    UINT32 i;

    HWI_PROC_FUNC *hwiForm = (HWI_PROC_FUNC *)ArchGetHwiFrom();
    for (i = 1; i < OS_SYS_VECTOR_CNT; i++) {
        hwiForm[i] = (HWI_PROC_FUNC)HandleEntry;
    }

    for (i = OS_SYS_VECTOR_CNT; i < (LOSCFG_PLATFORM_HWI_LIMIT + OS_SYS_VECTOR_CNT); i++) {
        hwiForm[i] = (HWI_PROC_FUNC)IrqEntry;
    }
    HalHwiHandleReInit((UINT32)hwiForm);

    HalSetVbr((UINT32)hwiForm);
    for (i = 0; i < BYTES_OF_128_INT; i++) {
        VIC_REG->IABR[i] = 0x0;
        VIC_REG->ICPR[i] = MASK_32_BITS;
    }
    return;
}


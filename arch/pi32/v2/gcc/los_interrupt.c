/*
 *  Copyright (c) 2022 ZhuHai Jieli Technology Co.,Ltd.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "los_interrupt.h"
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
#include "hwi.h"
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
#include "los_cpup.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

UINT32 g_intCount = 0;

#ifdef __ICCARM__
#pragma location = ".data.vector"
#elif defined(__CC_ARM) || defined(__GNUC__)
#pragma data_alignment = LOSCFG_ARCH_HWI_VECTOR_ALIGN
LITE_OS_SEC_VEC
#endif
/* *
 * @ingroup los_hwi
 * Hardware interrupt form mapping handling function array.
 */
STATIC HWI_PROC_FUNC g_hwiForm[OS_VECTOR_CNT] = {0};


/* *
 * @ingroup los_hwi
 * hardware interrupt handler form mapping handling function array.
 */
STATIC HWI_PROC_FUNC g_hwiHandlerForm[OS_VECTOR_CNT] = {0};




/* ****************************************************************************
 Function    : HwiNumGet
 Description : Get an interrupt number
 Input       : None
 Output      : None
 Return      : Interrupt Indexes number
 **************************************************************************** */
STATIC UINT32 HwiNumGet(VOID)
{
    UINT32 icfg = 0;
    __asm__ volatile("%0 = icfg" : "=r"(icfg));

    return ((icfg & 0xff0000) >> 16);

    /* return __get_IPSR(); */
}


/* ****************************************************************************
 Function    : HalHwiDefaultHandler
 Description : default handler of the hardware interrupt
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT_MINOR VOID HalHwiDefaultHandler(VOID)
{
    PRINT_ERR("%s irqnum:%u\n", __FUNCTION__, HwiNumGet());
    while (1) {}
}

WEAK VOID HalPreInterruptHandler(UINT32 arg)
{
    (VOID)arg;
    return;
}

WEAK VOID HalAftInterruptHandler(UINT32 arg)
{
    (VOID)arg;
    return;
}




/* ****************************************************************************
 Function    : HalInterrupt
 Description : Hardware interrupt entry function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
__attribute__((interrupt(""))) 
LITE_OS_SEC_TEXT VOID HalInterrupt(VOID)
{
    UINT32 hwiIndex;
    UINT32 intSave;


    intSave = LOS_IntLock();
    g_intCount++;
    LOS_IntRestore(intSave);

    hwiIndex = HwiNumGet();

    OsHookCall(LOS_HOOK_TYPE_ISR_ENTER, hwiIndex);
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
    OsCpupIrqStart(hwiIndex);
#endif

    HalPreInterruptHandler(hwiIndex);

    if (g_hwiHandlerForm[hwiIndex] != 0) {
        g_hwiHandlerForm[hwiIndex]();
    }


    HalAftInterruptHandler(hwiIndex);

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
    OsCpupIrqEnd(hwiIndex);
#endif

    OsHookCall(LOS_HOOK_TYPE_ISR_EXIT, hwiIndex);

    intSave = LOS_IntLock();
    g_intCount--;
    LOS_IntRestore(intSave);
}









/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector)
{
    if ((num + OS_SYS_VECTOR_CNT) < OS_VECTOR_CNT) {
        g_hwiForm[num + OS_SYS_VECTOR_CNT] = HalInterrupt;
        g_hwiHandlerForm[num + OS_SYS_VECTOR_CNT] = vector;
    }
}

STATIC UINT32 HwiUnmask(HWI_HANDLE_T hwiNum)
{
	/* printf("=>%s\n",__func__); */
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
	bit_set_ie(hwiNum);
    /* NVIC_EnableIRQ((IRQn_Type)hwiNum); */

    return LOS_OK;
}

STATIC UINT32 HwiMask(HWI_HANDLE_T hwiNum)
{
	/* printf("=>%s\n",__func__); */
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    /* NVIC_DisableIRQ((IRQn_Type)hwiNum); */

	bit_clr_ie(hwiNum);
    return LOS_OK;
}

STATIC UINT32 HwiSetPriority(HWI_HANDLE_T hwiNum, UINT8 priority)
{
	/* printf("=>%s\n",__func__); */
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    if (priority > OS_HWI_PRIO_LOWEST) {
        return OS_ERRNO_HWI_PRIO_INVALID;
    }
	reg_set_ip(hwiNum,priority);
    /* NVIC_SetPriority((IRQn_Type)hwiNum, priority); */

    return LOS_OK;
}

STATIC UINT32 HwiPending(HWI_HANDLE_T hwiNum)
{

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
	// only use in soft interrupt
	
    q32DSP(0)->ILAT_SET |= 1 << (hwiNum - 60);


    /* NVIC_SetPendingIRQ((IRQn_Type)hwiNum); */

    return LOS_OK;
}

STATIC UINT32 HwiClear(HWI_HANDLE_T hwiNum)
{
	/* printf("=>%s\n",__func__); */
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    q32DSP(0)->ILAT_CLR = 1<<(hwiNum - 60);
    /* NVIC_ClearPendingIRQ((IRQn_Type)hwiNum); */

    return LOS_OK;
}

HwiControllerOps g_archHwiOps = {
    .enableIrq      = HwiUnmask,
    .disableIrq     = HwiMask,
    .setIrqPriority = HwiSetPriority,
    .getCurIrqNum   = HwiNumGet,
    .triggerIrq     = HwiPending,
    .clearIrq       = HwiClear,
};

inline UINT32 ArchIsIntActive(VOID)
{

    UINT32 icfg = 0;
    __asm__ volatile("%0 = icfg" : "=r"(icfg));
    return ((icfg & 0xff));
}
/* ****************************************************************************
 Function    : ArchHwiCreate
 Description : create hardware interrupt
 Input       : hwiNum   --- hwi num to create
               hwiPrio  --- priority of the hwi
               hwiMode  --- unused
               hwiHandler --- hwi handler
               irqParam --- param of the hwi handler
 Output      : None
 Return      : LOS_OK on success or error code on failure
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT UINT32 ArchHwiCreate(HWI_HANDLE_T hwiNum,
                                           HWI_PRIOR_T hwiPrio,
                                           HWI_MODE_T hwiMode,
                                           HWI_PROC_FUNC hwiHandler,
                                           HwiIrqParam *irqParam)
{
    (VOID)hwiMode;
    UINT32 intSave;

    if (hwiHandler == NULL) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

	/* printf("%s   %d\n",__func__,__LINE__); */
	if (g_hwiForm[hwiNum + OS_SYS_VECTOR_CNT] != (HWI_PROC_FUNC)HalHwiDefaultHandler) {
		return OS_ERRNO_HWI_ALREADY_CREATED;
	}

	/* printf("%s   %d\n",__func__,__LINE__); */
    /* if (hwiPrio > OS_HWI_PRIO_LOWEST) { */
        /* return OS_ERRNO_HWI_PRIO_INVALID; */
    /* } */

    intSave = LOS_IntLock();
    (VOID)irqParam;
    OsSetVector(hwiNum, hwiHandler);


    HwiUnmask(hwiNum);
    HwiSetPriority(hwiNum, hwiPrio);

	/* printf("g_hwiForm:%x\n",g_hwiForm[hwiNum + OS_SYS_VECTOR_CNT]); */
    request_irq(hwiNum, hwiPrio, g_hwiForm[hwiNum+OS_SYS_VECTOR_CNT], 0);

    LOS_IntRestore(intSave);

    return LOS_OK;
}

/* ****************************************************************************
 Function    : ArchHwiDelete
 Description : Delete hardware interrupt
 Input       : hwiNum   --- hwi num to delete
               irqParam --- param of the hwi handler
 Output      : None
 Return      : LOS_OK on success or error code on failure
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT UINT32 ArchHwiDelete(HWI_HANDLE_T hwiNum, HwiIrqParam *irqParam)
{
    (VOID)irqParam;
    UINT32 intSave;

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    HwiMask(hwiNum);

    intSave = LOS_IntLock();

    g_hwiForm[hwiNum + OS_SYS_VECTOR_CNT] = (HWI_PROC_FUNC)HalHwiDefaultHandler;
    unrequest_irq(hwiNum);

    LOS_IntRestore(intSave);

    return LOS_OK;
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

    interrupt_init();
    /* int cpu = 0; */
    /* task switch interrupt */
    request_irq(IRQ_SOFT3_IDX, 0, ArchPendSV, 0);
    /* initialize timer interrupt */
	int index;

/* #if (LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT == 1) */
    /* UINT32 index; */
    /* g_hwiForm[0] = 0;             [> [0] Top of Stack <] */
    /* g_hwiForm[1] = (HWI_PROC_FUNC)Reset_Handler; [> [1] reset <] */
	for (index = 0; index < OS_VECTOR_CNT; index++) { /* 2: The starting position of the interrupt */
		g_hwiForm[index] = (HWI_PROC_FUNC)HalHwiDefaultHandler;
		/* request_irq(index, 0, ArchPendSV, 0); */
	}
    /* Exception handler register */
    /* g_hwiForm[NonMaskableInt_IRQn + OS_SYS_VECTOR_CNT]   = (HWI_PROC_FUNC)HalExcNMI; */
    /* g_hwiForm[HARDFAULT_IRQN + OS_SYS_VECTOR_CNT]        = (HWI_PROC_FUNC)HalExcHardFault; */
    /* g_hwiForm[MemoryManagement_IRQn + OS_SYS_VECTOR_CNT] = (HWI_PROC_FUNC)HalExcMemFault; */
    /* g_hwiForm[BusFault_IRQn + OS_SYS_VECTOR_CNT]         = (HWI_PROC_FUNC)HalExcBusFault; */
    /* g_hwiForm[UsageFault_IRQn + OS_SYS_VECTOR_CNT]       = (HWI_PROC_FUNC)HalExcUsageFault; */
    /* g_hwiForm[SVCall_IRQn + OS_SYS_VECTOR_CNT]           = (HWI_PROC_FUNC)HalExcSvcCall; */
    /* g_hwiForm[PendSV_IRQn + OS_SYS_VECTOR_CNT]           = (HWI_PROC_FUNC)HalPendSV; */
    /* g_hwiForm[SysTick_IRQn + OS_SYS_VECTOR_CNT]          = (HWI_PROC_FUNC)SysTick_Handler; */

    /* Interrupt vector table location */
    /* SCB->VTOR = (UINT32)(UINTPTR)g_hwiForm; */
/* #endif */
/* #if (__CORTEX_M >= 0x03U) [> only for Cortex-M3 and above <] */
    /* NVIC_SetPriorityGrouping(OS_NVIC_AIRCR_PRIGROUP); */
/* #endif */

    /* Enable USGFAULT, BUSFAULT, MEMFAULT */
    /* *(volatile UINT32 *)OS_NVIC_SHCSR |= (USGFAULT | BUSFAULT | MEMFAULT); */

    /* Enable DIV 0 and unaligned exception */
/* #ifdef LOSCFG_ARCH_UNALIGNED_EXC */
    /* *(volatile UINT32 *)OS_NVIC_CCR |= (DIV0FAULT | UNALIGNFAULT); */
/* #else */
    /* *(volatile UINT32 *)OS_NVIC_CCR |= (DIV0FAULT); */
/* #endif */

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
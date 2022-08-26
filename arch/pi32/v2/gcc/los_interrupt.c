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

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

LITE_OS_SEC_BSS UINT32  g_intCount = 0;


LITE_OS_SEC_TEXT_INIT VOID ArchHwiInit()
{
    interrupt_init();
    int cpu = 0;
    /* task switch interrupt */
    request_irq(IRQ_SOFT3_IDX, 0, ArchPendSV, 0);
    /* initialize timer interrupt */
}

UINT32 ArchIsIntActive(VOID)
{
    UINT32 icfg = 0;
    __asm__ volatile("%0 = icfg" : "=r"(icfg));
    return (icfg & 0xff) != 0;
}

UINT32 ArchHwiCreate(HWI_HANDLE_T hwiNum,
                     HWI_PRIOR_T hwiPrio,
                     HWI_MODE_T mode,
                     HWI_PROC_FUNC handler,
                     HwiIrqParam *irqParam)
{
    UINT32 intSave;

    (VOID) irqParam;
    if (handler == NULL) {
        return OS_ERROR;
    }
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERROR;
    }
    if ((hwiPrio < OS_HWI_PRIO_LOWEST) || (hwiPrio > OS_HWI_PRIO_HIGHEST)) {
        return OS_ERROR;
    }

    request_irq(hwiNum, hwiPrio, handler, 0);

    return LOS_OK;
}

UINT32 ArchHwiDelete(HWI_HANDLE_T hwiNum, HwiIrqParam *irqParam)
{
    UINT32 intSave;

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERROR;
    }

    unrequest_irq(hwiNum);

    return LOS_OK;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

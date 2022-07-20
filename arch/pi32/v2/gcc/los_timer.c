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

#include "los_timer.h"
#include "los_config.h"
#include "los_tick.h"
#include "los_arch_interrupt.h"
#include "los_arch_timer.h"
#include "los_debug.h"

#include "csfr.h"

#define IRQ_TICK_TMR_IDX   3
#define TICK_CLEAR_PENDIND   (1UL << 6)
#define TICK_START           (1UL << 0)

static HWI_PROC_FUNC systick_handler = (HWI_PROC_FUNC)NULL;

STATIC UINT32 SysTickStart(HWI_PROC_FUNC handler);
STATIC UINT64 SysTickReload(UINT64 nextResponseTime);
STATIC UINT64 SysTickCycleGet(UINT32 *period);
STATIC VOID SysTickLock(VOID);
STATIC VOID SysTickUnlock(VOID);

STATIC ArchTickTimer g_archTickTimer = {
    .freq = 0,
    .irqNum = IRQ_TICK_TMR_IDX,
    .periodMax = LOSCFG_BASE_CORE_TICK_RESPONSE_MAX,
    .init = SysTickStart,
    .getCycle = SysTickCycleGet,
    .reload = SysTickReload,
    .lock = SysTickLock,
    .unlock = SysTickUnlock,
    .tickHandler = NULL,
};

__attribute__((interrupt(""))) static void tickISR(void)
{
    TICK_CON |= TICK_CLEAR_PENDIND;
    jiffies++;
    if (systick_handler != NULL) {
        systick_handler();
    }
}

/* ****************************************************************************
Function    : HalTickStart
Description : Configure Tick Interrupt Start
Input       : none
output      : none
return      : LOS_OK - Success , or LOS_ERRNO_TICK_CFG_INVALID - failed
**************************************************************************** */
STATIC UINT32 SysTickStart(HWI_PROC_FUNC handler)
{
    ArchTickTimer *tick = &g_archTickTimer;

    tick->freq = OS_SYS_CLOCK;

    systick_handler = handler;
    TICK_CON = 0;
    ArchHwiCreate(IRQ_TICK_TMR_IDX, 1, 0, tickISR, 0);

    TICK_CNT = 0;
    TICK_PRD = (OS_CYCLE_PER_TICK - 1);
    TICK_CON = TICK_START;

    return LOS_OK;
}

STATIC UINT64 SysTickReload(UINT64 nextResponseTime)
{
    if (nextResponseTime > g_archTickTimer.periodMax) {
        nextResponseTime = g_archTickTimer.periodMax;
    }

    TICK_CON &= ~TICK_START;
    TICK_PRD = (UINT32)(nextResponseTime - 1UL); /* set reload register */
    TICK_CNT = 0UL; /* Load the SysTick Counter Value */
    TICK_CON |= TICK_CLEAR_PENDIND;
    TICK_CON |= TICK_START;

    return nextResponseTime;
}

STATIC UINT64 SysTickCycleGet(UINT32 *period)
{
    UINT32 hwCycle;
    UINT32 intSave = LOS_IntLock();
    *period = TICK_PRD;
    hwCycle = TICK_CNT;
    LOS_IntRestore(intSave);
    return (UINT64)hwCycle;
}

STATIC VOID SysTickLock(VOID)
{
    TICK_CON &= ~TICK_START;
}

STATIC VOID SysTickUnlock(VOID)
{
    TICK_CON |= TICK_START;
}

ArchTickTimer *ArchSysTickTimerGet(VOID)
{
    return &g_archTickTimer;
}

VOID Wfi(VOID)
{
}

VOID Dsb(VOID)
{
}

UINT32 ArchEnterSleep(VOID)
{
    Dsb();
    Wfi();
    return LOS_OK;
}

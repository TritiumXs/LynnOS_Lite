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

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

#include "los_timer.h"
#include "los_config.h"
#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_reg.h"

/* Register required to configure the Real-Time Interrupt (RTI) */
#define RTI_REG_BASE                0xFFFFFC00UL
#define RTI_GCTRL_REG               (RTI_REG_BASE)        /* RTI Global Control Register */
#define RTI_TBCTRL_REG              (RTI_REG_BASE + 0x04) /* RTI Timebase Control Register */
#define RTI_COMPCTRL_REG            (RTI_REG_BASE + 0x0C) /* RTI Compare Control Register */
#define RTI_FRC0_REG                (RTI_REG_BASE + 0x10) /* RTI Free Running Counter 0 Register */
#define RTI_UC0_REG                 (RTI_REG_BASE + 0x14) /* RTI Up Counter 0 Register */
#define RTI_CPUC0_REG               (RTI_REG_BASE + 0x18) /* RTI Compare Up Counter 0 Register */
#define RTI_COMP0_REG               (RTI_REG_BASE + 0x50) /* RTI Compare 0 Register */
#define RTI_UDCP0_REG               (RTI_REG_BASE + 0x54) /* RTI Update Compare 0 Register */
#define RTI_SETINTENA_REG           (RTI_REG_BASE + 0x80) /* RTI Set Interrupt Enable Register */
#define RTI_CLEARINTENA_REG         (RTI_REG_BASE + 0x84) /* RTI Clear Interrupt Enable Register */
#define RTI_INTFLAG_REG             (RTI_REG_BASE + 0x88) /* RTI Interrupt Flag Register */

#define RTI_IRQ_NUM             2U
#define RTI_CLOCK_HZ            75000000U
#define RTI_ENABLE_BIT          0x00000001U
#define RTI_INT_ENABLE_BIT      0x00000001U
#define RTI_INT_DISABLE_BIT     0x00000001U
#define RTI_INT_CLEAR_BIT       0x00000001U

STATIC UINT32 SysTickStart(HWI_PROC_FUNC handler);
STATIC UINT64 SysTickReload(UINT64 nextResponseTime);
STATIC UINT64 SysTickCycleGet(UINT32 *period);
STATIC VOID SysTickLock(VOID);
STATIC VOID SysTickUnlock(VOID);

STATIC ArchTickTimer g_archTickTimer = {
    .freq = 0,
    .irqNum = RTI_IRQ_NUM,
    .periodMax = LOSCFG_BASE_CORE_TICK_RESPONSE_MAX,
    .init = SysTickStart,
    .getCycle = SysTickCycleGet,
    .reload = SysTickReload,
    .lock = SysTickLock,
    .unlock = SysTickUnlock,
    .tickHandler = NULL,
};

STATIC UINT32 SysTickStart(HWI_PROC_FUNC handler)
{
    UINT32 intSave = LOS_IntLock();
    UINT32 value = 0;

    ArchTickTimer *tick = &g_archTickTimer;
    tick->freq = OS_SYS_CLOCK;

    (VOID)ArchHwiCreate(tick->irqNum, 0, 0, (HWI_PROC_FUNC)handler, 0);

    /* Disable timer 0 */
    READ_UINT32(value, RTI_GCTRL_REG);
    value &= ~(RTI_ENABLE_BIT);
    WRITE_UINT32(value, RTI_GCTRL_REG);

    /* Use the internal counter */
    WRITE_UINT32(0, RTI_TBCTRL_REG);

    /* COMPSEL0 will use the RTIFRC0 counter */
    WRITE_UINT32(0, RTI_COMPCTRL_REG);

    /* Initialise the counter and the prescale counter register */
    WRITE_UINT32(0, RTI_UC0_REG);
    WRITE_UINT32(0, RTI_FRC0_REG);

    /* Set prescalar for RTI clock */
    value = (RTI_CLOCK_HZ / OS_SYS_CLOCK) - 1;
    WRITE_UINT32(value, RTI_CPUC0_REG);
    value = OS_SYS_CLOCK / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    WRITE_UINT32(value, RTI_COMP0_REG);
    value = OS_SYS_CLOCK / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    WRITE_UINT32(value, RTI_UDCP0_REG);

    /*  Clear interrupts */
    value = RTI_INT_DISABLE_BIT;
    WRITE_UINT32(value, RTI_CLEARINTENA_REG);
    value = RTI_INT_CLEAR_BIT;
    WRITE_UINT32(value, RTI_INTFLAG_REG);

    /* Enable the compare 0 interrupt */
    READ_UINT32(value, RTI_SETINTENA_REG);
    value |= RTI_INT_ENABLE_BIT;
    WRITE_UINT32(value, RTI_SETINTENA_REG);

    READ_UINT32(value, RTI_GCTRL_REG);
    value |= RTI_ENABLE_BIT;
    WRITE_UINT32(value, RTI_GCTRL_REG);

    LOS_IntRestore(intSave);

    return LOS_OK;
}

STATIC VOID SysTickClockIrqClear(VOID)
{
    UINT32 value = RTI_INT_CLEAR_BIT;
    WRITE_UINT32(value, RTI_INTFLAG_REG);
}

STATIC UINT64 SysTickReload(UINT64 nextResponseTime)
{
    UINT32 rtiCmpComp = 0;
    UINT32 rtiCntFrc = 0;
    if (nextResponseTime > g_archTickTimer.periodMax) {
        nextResponseTime = g_archTickTimer.periodMax;
    }

    SysTickLock();
    WRITE_UINT32((UINT32)nextResponseTime, RTI_UDCP0_REG);
    READ_UINT32(rtiCntFrc, RTI_FRC0_REG);
    rtiCmpComp = rtiCntFrc + (UINT32)nextResponseTime;
    WRITE_UINT32(rtiCmpComp, RTI_COMP0_REG);

    SysTickClockIrqClear();
    SysTickUnlock();
    return nextResponseTime;
}

STATIC UINT64 SysTickCycleGet(UINT32 *period)
{
    UINT32 val = 0;
    UINT32 rtiCmpComp = 0;
    UINT32 rtiCntFrc = 0;
    UINT32 rtiCmpUdcp = 0;

    READ_UINT32(rtiCmpComp, RTI_COMP0_REG);
    READ_UINT32(rtiCntFrc, RTI_FRC0_REG);
    READ_UINT32(rtiCmpUdcp, RTI_UDCP0_REG);

    val = rtiCntFrc - (rtiCmpComp - rtiCmpUdcp);
    *period = rtiCmpUdcp;

    return (UINT64)val;
}

STATIC VOID SysTickLock(VOID)
{
    UINT32 value = 0;

    READ_UINT32(value, RTI_GCTRL_REG);
    value &= ~(RTI_ENABLE_BIT);
    WRITE_UINT32(value, RTI_GCTRL_REG);
}

STATIC VOID SysTickUnlock(VOID)
{
    UINT32 value = 0;

    READ_UINT32(value, RTI_GCTRL_REG);
    value |= RTI_ENABLE_BIT;
    WRITE_UINT32(value, RTI_GCTRL_REG);
}

ArchTickTimer *ArchSysTickTimerGet(VOID)
{
    return &g_archTickTimer;
}

UINT32 ArchEnterSleep(VOID)
{
    dsb();
    wfi();
    isb();

    return LOS_OK;
}

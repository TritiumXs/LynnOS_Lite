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

#include <stdio.h>
#include "los_timer.h"
#include "los_config.h"
#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_reg.h"

#define OS_TIMER_IRQ_NUM            TIMER_IRQ

#define OS_TIMER_EN                 (0x00000080)
#define OS_TIMER_MODE_PERIODIC      (0x00000040)
#define OS_TIMER_INTR_ENABLE        (0x00000020)
#define OS_TIMER_PRE_1              (0x00000008)
#define OS_TIMER_PRE_2              (0x00000004)
#define OS_TIMER_SIZE               (0x00000002)
#define OS_TIMER_ONE_SHOT           (0x00000001)

#define OS_TIMER_REG_BASE           TIMER_BASE
#define OS_TIMER_LOAD_ADDR          (OS_TIMER_REG_BASE) /* Load Register, Timer Load */
#define OS_TIMER_VALUE_ADDR         (OS_TIMER_REG_BASE + 0x04) /* Current Value Register, Timer Value, read only */
#define OS_TIMER_CONTROL_ADDR       (OS_TIMER_REG_BASE + 0x08) /* Control Register, Timer Control */
#define OS_TIMER_INTCLR_ADDR        (OS_TIMER_REG_BASE + 0x0C) /* Interrupt Clear Register, write only */

STATIC UINT32 SysTickStart(HWI_PROC_FUNC handler);
STATIC UINT64 SysTickReload(UINT64 nextResponseTime);
STATIC UINT64 SysTickCycleGet(UINT32 *period);
STATIC VOID SysTickLock(VOID);
STATIC VOID SysTickUnlock(VOID);

STATIC ArchTickTimer g_archTickTimer = {
    .freq = 0,
    .irqNum = OS_TIMER_IRQ_NUM,
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
    UINT32 value;
    UINT32 ticks;
    UINT32 ret;

    ArchTickTimer *tick = &g_archTickTimer;
    tick->freq = OS_SYS_CLOCK;

    READ_UINT32(value, OS_TIMER_CONTROL_ADDR);
    value |= (OS_TIMER_MODE_PERIODIC | OS_TIMER_SIZE);
    WRITE_UINT32(value, OS_TIMER_CONTROL_ADDR);

    READ_UINT32(value, OS_TIMER_CONTROL_ADDR);
    value &= ~(OS_TIMER_EN | OS_TIMER_INTR_ENABLE | OS_TIMER_PRE_1 | OS_TIMER_PRE_2 | OS_TIMER_ONE_SHOT);
    WRITE_UINT32(value, OS_TIMER_CONTROL_ADDR);

    ticks = OS_SYS_CLOCK / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    WRITE_UINT32(ticks, OS_TIMER_LOAD_ADDR);

    value |= OS_TIMER_INTR_ENABLE;
    WRITE_UINT32(value, OS_TIMER_CONTROL_ADDR);

    value |= OS_TIMER_EN;
    WRITE_UINT32(value, OS_TIMER_CONTROL_ADDR);

    ret = ArchHwiCreate(OS_TIMER_IRQ_NUM, 0, 0, (HWI_PROC_FUNC)handler, 0);
    if (ret != LOS_OK) {
        return ret;
    }
    LOS_IntRestore(intSave);

    return LOS_OK;
}

STATIC VOID SysTickClockIrqClear(VOID)
{
    UINT32 value = 0xFFFFFFFF;
    WRITE_UINT32(value, OS_TIMER_INTCLR_ADDR);
}

STATIC UINT64 SysTickReload(UINT64 nextResponseTime)
{
    UINT32 value;

    if (nextResponseTime > g_archTickTimer.periodMax) {
        nextResponseTime = g_archTickTimer.periodMax;
    }

    SysTickLock();
    value = (UINT32)(nextResponseTime);
    WRITE_UINT32(value, OS_TIMER_LOAD_ADDR);
    READ_UINT32(value, OS_TIMER_CONTROL_ADDR);
    value |= OS_TIMER_ONE_SHOT;
    WRITE_UINT32(value, OS_TIMER_CONTROL_ADDR);
    SysTickClockIrqClear();
    SysTickUnlock();
    return nextResponseTime;
}

STATIC UINT64 SysTickCycleGet(UINT32 *period)
{
    UINT32 val = 0;

    UINT32 intSave = LOS_IntLock();
    *period = GET_UINT32(OS_TIMER_LOAD_ADDR);
    val = GET_UINT32(OS_TIMER_VALUE_ADDR);
    if (val != 0) {
        val = *period - val;
    }
    LOS_IntRestore(intSave);
    return (UINT64)val;
}

STATIC VOID SysTickLock(VOID)
{
    UINT32 value;
    READ_UINT32(value, OS_TIMER_CONTROL_ADDR);
    value &= ~OS_TIMER_EN;
    WRITE_UINT32(value, OS_TIMER_CONTROL_ADDR);
}

STATIC VOID SysTickUnlock(VOID)
{
    UINT32 value;
    READ_UINT32(value, OS_TIMER_CONTROL_ADDR);
    value |= OS_TIMER_EN;
    WRITE_UINT32(value, OS_TIMER_CONTROL_ADDR);
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

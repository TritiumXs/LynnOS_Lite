/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "riscv_hal.h"
#include "los_debug.h"
#include "soc.h"
#include "los_context.h"
#include "los_task.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*------------------------------------------------------------*/
/* declare functions */
/*------------------------------------------------------------*/
typedef int intptr_t;
typedef void (*intr_handler_t)(void *arg);

extern BOOL g_taskScheduled;
extern VOID LosSetTimerVal(UINT64 time);
extern VOID LosGetTimerVal(UINT32* timerL, UINT32* timerH);
/*------------------------------------------------------------*/
/* Trace Related functions */
/*------------------------------------------------------------*/
BOOL HalBackTraceFpCheck(UINT32 value)
{
    if (value >= (UINT32)(UINTPTR)(&__bss_end)) {
        return TRUE;
    }

    if ((value >= (UINT32)(UINTPTR)(&__start_and_irq_stack_top)) && (value < (UINT32)(UINTPTR)(&__except_stack_top))) {
        return TRUE;
    }

    return FALSE;
}

BOOL HalBackTraceRaCheck(UINT32 value)
{
    BOOL ret = FALSE;

    if ((value >= (UINT32)(UINTPTR)(&__text_start)) &&
        (value < (UINT32)(UINTPTR)(&__text_end))) {
        ret = TRUE;
    }

    return ret;
}

/*------------------------------------------------------------*/
/* Timer Related functions */
/*------------------------------------------------------------*/
VOID HalClockInit(OS_TICK_HANDLER handler, UINT32 period)
{
    // NO USE MTIMER;USE RTC TIMER
    return;
}

VOID HalSysTickSet(UINT64 nextRespTime) 
{
    LosSetTimerVal(nextRespTime);
}

VOID HalSysTickGet(UINT32* timerL, UINT32* timerH) 
{
    LosGetTimerVal(timerL, timerH);
}

VOID HalSysTickReload(UINT64 nextResponseTime)
{
    HalSysTickSet(nextResponseTime);  
}

UINT64 HalGetTickCycle(UINT32 *period)
{
    (VOID)period;
    UINT32 timerL = 0, timerH = 0;

    HalSysTickGet(&timerL, &timerH);
    return (((UINT64)(timerH) << 32) | (timerL));     
}

/*------------------------------------------------------------*/
/* Isr Related functions */
/*------------------------------------------------------------*/
__attribute__((section(".exception_vectors.text")))
VOID HalTaskSched(VOID)
{
    UINT32 intSave;

    if (OS_INT_ACTIVE) {
        return;
    }

    if (g_taskScheduled && LOS_CHECK_SCHEDULE) {
        intSave = LOS_IntLock();
        BOOL isSwitch = OsSchedTaskSwitch();
        if (isSwitch) {
            HalTaskContextSwitch1(intSave);
            return;
        }
        LOS_IntRestore(intSave);
    }

    return;
}

__attribute__((section(".exception_vectors.text")))
VOID HalIrqEndCheckSched(VOID)
{
    if (OS_INT_ACTIVE) {
        return;
    }
    HalTaskSched();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

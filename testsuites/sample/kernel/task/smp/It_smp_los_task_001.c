/*
* Copyright (c) 2023 Hunan OpenValley Digital Industry Development Co., Ltd.
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

#include "osTest.h"
#include "It_los_task.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

static SPIN_LOCK_INIT(g_TestSpin);

static void TaskF01(void)
{
    UINT32 puvIntSave;

    LOS_SpinLockSave(&g_TestSpin, &puvIntSave);
    g_testCount |= 1 << ArchCurrCpuid();
    LOS_SpinUnlockRestore(&g_TestSpin, puvIntSave);
}

static UINT32 Testcase(void)
{
    TSK_INIT_PARAM_S testTask;
    UINT32 ret;
    UINT32 testid;
    UINT32 coreIdx = 0;

    g_testCount = 0;

    while (coreIdx < LOSCFG_KERNEL_CORE_NUM) {
        TEST_TASK_PARAM_INIT_AFFI(testTask, "it_smp_task_001", TaskF01, TASK_PRIO_TEST + 1,
            CPUID_TO_AFFI_MASK(coreIdx));
        ret = LOS_TaskCreate(&testid, &testTask);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
        coreIdx++;
    }

    LOS_TaskDelay(10); // 10, set delay time.

    ICUNIT_ASSERT_EQUAL(g_testCount, LOSCFG_KERNEL_CPU_MASK, g_testCount);

    return LOS_OK;
}

void ItSmpLosTask001(void)
{
    TEST_ADD_CASE("ItSmpLosTask001", Testcase, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

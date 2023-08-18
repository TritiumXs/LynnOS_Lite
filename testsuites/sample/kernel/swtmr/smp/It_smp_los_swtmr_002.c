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
#include "It_los_swtmr.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

static UINT32 g_testTimes;

static UINT32 g_swtmrHandle[LOSCFG_KERNEL_CORE_NUM];

static VOID SwtmrF01(void)
{
    LOS_AtomicInc(&g_testCount);
}

static void TaskF01(UINTPTR swtmrId)
{
    UINT32 ret;

    ret = LOS_SwtmrStart(swtmrId);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    

    return;

EXIT:
    LOS_SwtmrDelete(swtmrId);
    return;
}

static UINT32 Testcase(void)
{
    TSK_INIT_PARAM_S testTask;
    UINT32 ret, testid;
    UINT32 i;

    /* each core run swtmr for 10 times, period is 10 */
    g_testCount = 0;
    g_testPeriod = 10; // period is 10
    g_testTimes = 1;

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        ret = LOS_SwtmrCreate(g_testPeriod, LOS_SWTMR_MODE_PERIOD, (SWTMR_PROC_FUNC)SwtmrF01, &g_swtmrHandle[i], 0
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
        , OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_INSENSITIVE
#endif
        );
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        TEST_TASK_PARAM_INIT(testTask, "it_swtmr_002_task", TaskF01,
            TASK_PRIO_TEST_SWTMR + 1); // not set cpuaffi
        testTask.uwArg = g_swtmrHandle[i];
        testTask.usCpuAffiMask = CPUID_TO_AFFI_MASK(i);
        ret = LOS_TaskCreate(&testid, &testTask);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }

    UINT64 startTime = LOS_CurrNanosec();
    LOS_TaskDelay(g_testPeriod * g_testTimes + 5); // g_testPeriod * g_testTimes + 5, set delay time
    UINT64 usedTime = LOS_CurrNanosec() - startTime;

    ICUNIT_ASSERT_EQUAL(g_testCount, g_testTimes * LOSCFG_KERNEL_CORE_NUM, g_testCount);

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        ret = LOS_SwtmrStop(g_swtmrHandle[i]);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

        ret = LOS_SwtmrDelete(g_swtmrHandle[i]);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    }
    return LOS_OK;
EXIT:
    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        LOS_SwtmrDelete(g_swtmrHandle[i]);
    }
    return LOS_OK;
}

VOID ItSmpLosSwtmr002(VOID)
{
    TEST_ADD_CASE("ItSmpLosSwtmr002", Testcase, TEST_LOS, TEST_SWTMR, TEST_LEVEL1, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

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

static UINT32 g_szId[LOSCFG_KERNEL_CORE_NUM] = {0};
static UINT32 g_testSwtmtCount = 0;
static VOID SwtmrF01(void)
{
    LOS_AtomicInc(&g_testSwtmtCount);
}

static void TaskF01(UINT32 index)
{
    UINT32 ret;

    ret = LOS_SwtmrCreate(g_testPeriod, LOS_SWTMR_MODE_PERIOD, (SWTMR_PROC_FUNC)SwtmrF01, &g_szId[index], 0
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    , OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_INSENSITIVE
#endif
    );
    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);
    return;
}

static UINT32 Testcase(void)
{
    TSK_INIT_PARAM_S testTask;
    UINT32 ret;
    UINT32 currCpuid;
    UINT32 i;
    UINT32 taskID[LOSCFG_KERNEL_CORE_NUM];
    g_testSwtmtCount = 0;
    g_testPeriod = 10; // period is 10

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        TEST_TASK_PARAM_INIT_AFFI(testTask, "it_swtmr_025_task1", TaskF01, TASK_PRIO_TEST_SWTMR + 1, CPUID_TO_AFFI_MASK(i));
        testTask.uwArg = i;
        testTask.uwResved = LOS_TASK_ATTR_JOINABLE;
        ret = LOS_TaskCreate(&taskID[i], &testTask);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        ret = LOS_TaskJoin(taskID[i], NULL);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        ret = LOS_SwtmrStart(g_szId[i]);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }

    LOS_TaskDelay(g_testPeriod + 5); /* 5 ticks */

    ICUNIT_GOTO_EQUAL(g_testSwtmtCount, LOSCFG_KERNEL_CORE_NUM, g_testSwtmtCount, EXIT);

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        ret = LOS_SwtmrDelete(g_szId[i]);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }

    return LOS_OK;

EXIT:
    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        LOS_SwtmrDelete(g_szId[i]);
    }
    return LOS_OK;
}

VOID ItSmpLosSwtmr025(VOID)
{
    TEST_ADD_CASE("ItSmpLosSwtmr025", Testcase, TEST_LOS, TEST_SWTMR, TEST_LEVEL1, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

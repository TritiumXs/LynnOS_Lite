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

static VOID SwtmrF01(void)
{
    LOS_AtomicInc(&g_testCount);
}

static void TaskF01(void)
{
    UINT32 ret;
    LOS_AtomicInc(&g_testCount);
    ret = LOS_SwtmrStart(g_usSwTmrID);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    return;

EXIT:
    LOS_SwtmrDelete(g_usSwTmrID);
    return;
}

static UINT32 Testcase(void)
{
    TSK_INIT_PARAM_S testTask;
    UINT32 ret;
    UINT32 currCpuid;

    g_testCount = 0;
    g_testPeriod = 10; // period is 10

    ret = LOS_SwtmrCreate(g_testPeriod, LOS_SWTMR_MODE_ONCE, (SWTMR_PROC_FUNC)SwtmrF01, &g_usSwTmrID, 0
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    , OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_INSENSITIVE
#endif
    );
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    currCpuid = (ArchCurrCpuid() + 1) % (LOSCFG_KERNEL_CORE_NUM);
    TEST_TASK_PARAM_INIT_AFFI(testTask, "it_swtmr_004_task", TaskF01, TASK_PRIO_TEST_SWTMR - 1,
        CPUID_TO_AFFI_MASK(currCpuid)); // cross task
    ret = LOS_TaskCreate(&g_testTaskID01, &testTask);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    TestAssertBusyTaskDelay(100, 1); // 100, Set the timeout of runtime;
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

    LOS_TaskDelay(g_testPeriod + 1);

    ICUNIT_GOTO_EQUAL(g_testCount, 2, g_testCount, EXIT); // 2, The expected value

    /* do stop and delete */
    ret = LOS_SwtmrStop(g_usSwTmrID);
    ICUNIT_GOTO_NOT_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SwtmrDelete(g_usSwTmrID);
    ICUNIT_GOTO_NOT_EQUAL(ret, LOS_OK, ret, EXIT);
    return LOS_OK;

EXIT:
    LOS_SwtmrDelete(g_usSwTmrID);
    return LOS_OK;
}

VOID ItSmpLosSwtmr004(VOID)
{
    TEST_ADD_CASE("ItSmpLosSwtmr004", Testcase, TEST_LOS, TEST_SWTMR, TEST_LEVEL1, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

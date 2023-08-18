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

#include "It_los_mux.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

static VOID TaskF01(VOID)
{
    UINT32 ret;

    ICUNIT_ASSERT_EQUAL_VOID(g_testCount, 0, g_testCount);
    LOS_AtomicInc(&g_testCount);
    ret = LOS_MuxPend(g_mutexTest1, LOS_WAIT_FOREVER);
    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);
    LOS_AtomicInc(&g_testCount);
EXIT:
    ret = LOS_MuxPost(g_mutexTest1);
    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);
    LOS_AtomicInc(&g_testCount);
    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    TSK_INIT_PARAM_S testTask = {0};

    g_testCount = 0;

    ret = LOS_MuxCreate(&g_mutexTest1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_MuxPend(g_mutexTest1, LOS_WAIT_FOREVER);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    TEST_TASK_PARAM_INIT_AFFI(testTask, "it_mux_016_task1", TaskF01, 5 - 1,
        CPUID_TO_AFFI_MASK(ArchCurrCpuid())); // current cpu
    ret = LOS_TaskCreate(&g_testTaskID01, &testTask);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

    ret = LOS_MuxPost(g_mutexTest1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ICUNIT_GOTO_EQUAL(g_testCount, 3, g_testCount, EXIT); // 3, here assert the result.

    ret = LOS_MuxDelete(g_mutexTest1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

EXIT:
    LOS_TaskDelete(g_testTaskID01);
    LOS_MuxDelete(g_mutexTest1);
    return LOS_OK;
}


VOID ItSmpLosMux016(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItSmpLosMux016", Testcase, TEST_LOS, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

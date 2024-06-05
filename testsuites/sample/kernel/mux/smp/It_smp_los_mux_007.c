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


static void Task01(VOID)
{
    UINT32 ret;

    ret = LOS_MuxPend(g_mutexTest1, LOS_WAIT_FOREVER);
    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);

    TestBusyTaskDelay(5); // 5, delay for Timing control.

    ret = LOS_MuxPost(g_mutexTest1);
    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);

    g_testCount++;
}

static void Task02(VOID)
{
    UINT32 ret;

    ret = LOS_MuxPend(g_mutexTest1, LOS_WAIT_FOREVER);
    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);

    TestBusyTaskDelay(5); // 5, delay for Timing control.

    ret = LOS_MuxPost(g_mutexTest1);
    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);

    g_testCount++;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret, currCpuid, currCpuid2;
    g_testCount = 0;
    TSK_INIT_PARAM_S task = { 0 };
    TSK_INIT_PARAM_S task2 = { 0 };
    currCpuid = ArchCurrCpuid() % (LOSCFG_KERNEL_CORE_NUM - 1) + 1;
    currCpuid2 = currCpuid % (LOSCFG_KERNEL_CORE_NUM - 1) + 1;

    ret = LOS_MuxCreate(&g_mutexTest1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)Task01;
    task.pcName = "Test Case 1";
    task.uwStackSize = TASK_STACK_SIZE_TEST;
    task.uwResved = LOS_TASK_STATUS_DETACHED;
    task.usCpuAffiMask = CPUID_TO_AFFI_MASK(currCpuid);

    ret = LOS_TaskCreate(&g_testTaskID01, &task);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)Task02;
    task2.pcName = "Test Case 2";
    task2.uwStackSize = TASK_STACK_SIZE_TEST;
    task2.uwResved = LOS_TASK_STATUS_DETACHED;
    task2.usCpuAffiMask = CPUID_TO_AFFI_MASK(currCpuid2);

    ret = LOS_TaskCreate(&g_testTaskID02, &task2);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    TestAssertBusyTaskDelay(100, 2);                  // 100, 2, delay for Timing control.
    ICUNIT_ASSERT_EQUAL(g_testCount, 2, g_testCount); // 2, here assert the result.

    ret = LOS_MuxDelete(g_mutexTest1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

VOID ItSmpLosMux007(void)
{
    TEST_ADD_CASE("ItSmpLosMux007", Testcase, TEST_LOS, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
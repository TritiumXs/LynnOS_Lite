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
#include "It_los_sem.h"

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
    ret = LOS_SemPend(g_usSemID, LOS_WAIT_FOREVER);

    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);

    LOS_AtomicInc(&g_testCount);
    return;
}

static VOID TaskF02(VOID)
{
    UINT32 ret;

    ICUNIT_ASSERT_EQUAL_VOID(g_testCount, 1, g_testCount);

    ret = LOS_SemPost(g_usSemID);
    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);

    LOS_AtomicInc(&g_testCount);
    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    TSK_INIT_PARAM_S testTask;

    g_testCount = 0;

    ret = LOS_SemCreate(0, &g_usSemID);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    TEST_TASK_PARAM_INIT_AFFI(testTask, "it_sem_004_task1", TaskF01, TASK_PRIO_TEST - 1,
        CPUID_TO_AFFI_MASK(ArchCurrCpuid())); // current cpu
    ret = LOS_TaskCreate(&g_testTaskID01, &testTask);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    TEST_TASK_PARAM_INIT_AFFI(testTask, "it_sem_004_task2", TaskF02, TASK_PRIO_TEST - 1,
        CPUID_TO_AFFI_MASK(ArchCurrCpuid())); // current cpu
    ret = LOS_TaskCreate(&g_testTaskID02, &testTask);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_SemDelete(g_usSemID);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ICUNIT_GOTO_EQUAL(g_testCount, 3, g_testCount, EXIT); // 3, Here, assert that g_testCount is equal to

EXIT:
    LOS_TaskDelete(g_testTaskID01);
    LOS_TaskDelete(g_testTaskID02);
    LOS_SemDelete(g_usSemID);
    return LOS_OK;
}

VOID ItSmpLosSem004(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItSmpLosSem004", Testcase, TEST_LOS, TEST_SEM, TEST_LEVEL2, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


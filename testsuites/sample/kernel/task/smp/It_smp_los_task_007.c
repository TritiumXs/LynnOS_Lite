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

#include "It_los_task.h"
#include "los_atomic.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

static void TaskF01(void)
{
    g_testCount++;
}

static void TaskF02(void)
{
    g_testCount++;
}

static UINT32 Testcase(void)
{
    UINT32 ret;
    TSK_INIT_PARAM_S task1 = { 0 };
    UINT32 testTaskIDSmp[LOSCFG_KERNEL_CORE_NUM];

    g_testCount = 0;
     //It is used to calculate a priority relative to TASK_PRIO_TEST.
    TEST_TASK_PARAM_INIT_AFFI(task1, "it_smp_task_007", (TSK_ENTRY_FUNC)TaskF01, TASK_PRIO_TEST + 1, 0);
    ret = LOS_TaskCreate(&g_testTaskID01, &task1);
    /* give up this core to task_f01 of this core */
    LOS_TaskDelay(10); // 10, set delay time.

    
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
    /* take this core back to control, create an unbinded task */
    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF02;
    task1.usTaskPrio = TASK_PRIO_TEST + 2;
    task1.usCpuAffiMask = 0;
    ret = LOS_TaskCreate(&g_testTaskID02, &task1);
    LOS_TaskDelay(10); // 10, set delay time.

    /* take this core back to control */
    ICUNIT_ASSERT_EQUAL(g_testCount, 2, g_testCount);

    ret = LOS_TaskDelete(g_testTaskID01);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

void ItSmpLosTask007(void)
{
    TEST_ADD_CASE("ItSmpLosTask007", Testcase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

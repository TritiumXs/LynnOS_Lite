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
    while (1) {
    }
}

static void TaskF02Preempt(void)
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
    TEST_TASK_PARAM_INIT(task1, "it_smp_task_005", (TSK_ENTRY_FUNC)TaskF01, TASK_PRIO_TEST + 2);
    int i;
    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        /* take control of every cores */
        task1.usCpuAffiMask = CPUID_TO_AFFI_MASK(i);
        ret = LOS_TaskCreate(&testTaskIDSmp[i], &task1);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }

    /* give up this core to task_f01 of this core */
    LOS_TaskDelay(10); // 10, set delay time.

    /* take this core back to control, create an unbinded task */
    ICUNIT_ASSERT_EQUAL(g_testCount, LOSCFG_KERNEL_CORE_NUM, g_testCount);
    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF02Preempt;
    task1.usTaskPrio = TASK_PRIO_TEST + 1;
    task1.usCpuAffiMask = 0;
    ret = LOS_TaskCreate(&g_testTaskID01, &task1);
    LOS_TaskDelay(10); // 10, set delay time.
    /* take this core back to control */
    ICUNIT_ASSERT_EQUAL(g_testCount, LOSCFG_KERNEL_CORE_NUM + 1, g_testCount);
    
    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        ret = LOS_TaskDelete(testTaskIDSmp[i]);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }
    return LOS_OK;
}

void ItSmpLosTask005(void)
{
    TEST_ADD_CASE("ItSmpLosTask005", Testcase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

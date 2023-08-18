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

static void TaskF01(void)
{
    while (1) {
        LOS_TaskDelay(10);
    };
}

static UINT32 Testcase(void)
{
    TSK_INIT_PARAM_S testTask;
    UINT32 ret, currCpuid;
    UINT32 testid = 0xff;

    /* make sure that created test task is definitely on another core */
    currCpuid = (ArchCurrCpuid() + 1) % LOSCFG_KERNEL_CORE_NUM;

    TEST_TASK_PARAM_INIT_AFFI(testTask, "it_smp_task_003", TaskF01, OS_TASK_PRIORITY_LOWEST - 1,
        CPUID_TO_AFFI_MASK(currCpuid));

    ret = LOS_TaskCreate(&testid, &testTask);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    LOS_TaskDelay(2); // 2, set delay time.
    ret = LOS_TaskSuspend(testid);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* delay and check */
    LOS_TaskDelay(100); // 100, set delay time.

    ret = OS_TCB_FROM_TID(testid)->taskStatus;
    ICUNIT_GOTO_EQUAL(ret & OS_TASK_STATUS_SUSPEND, OS_TASK_STATUS_SUSPEND, ret, EXIT);

    LOS_TaskResume(testid);

EXIT:
    LOS_TaskDelay(2); // 2, set delay time.
    LOS_TaskDelete(testid);

    return LOS_OK;
}

void ItSmpLosTask003(void)
{
    TEST_ADD_CASE("ItSmpLosTask003", Testcase, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

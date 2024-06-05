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
#include "It_los_event.h"

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

    ret = LOS_EventRead(&g_pevent, 0x11, LOS_WAITMODE_AND, LOS_WAIT_FOREVER);
    ICUNIT_ASSERT_EQUAL_VOID(ret, g_pevent.uwEventID, ret);
    ICUNIT_ASSERT_EQUAL_VOID(g_pevent.uwEventID, 0x11, g_pevent.uwEventID);

    LOS_AtomicInc(&g_testCount);
    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    TSK_INIT_PARAM_S testTask;

    g_testCount = 0;
    g_pevent.uwEventID = 0;

    ret = LOS_EventInit(&g_pevent);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    TEST_TASK_PARAM_INIT_AFFI(testTask, "it_event_004_task", TaskF01, TASK_PRIO_TEST - 1,
        CPUID_TO_AFFI_MASK(ArchCurrCpuid()));
    ret = LOS_TaskCreate(&g_testTaskID01, &testTask);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_EventDestroy(&g_pevent);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_EVENT_SHOULD_NOT_DESTROYED, ret);

    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

EXIT:
    ret = LOS_TaskDelete(g_testTaskID01);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_EventDestroy(&g_pevent);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    return LOS_OK;
}

VOID ItSmpLosEvent004(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItSmpLosEvent004", Testcase, TEST_LOS, TEST_EVENT, TEST_LEVEL2, TEST_FUNCTION);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
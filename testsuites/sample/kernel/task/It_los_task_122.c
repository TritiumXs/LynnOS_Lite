/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "osTest.h"
#include "It_los_task.h"

static UINT32 g_joinTaskId;
static VOID *TaskJoinf01(void *argument)
{
    g_testCount++;

    return NULL;
}

static int TaskJoinf02(VOID *argument)
{
    UINT32 ret = LOS_TaskDelete(g_joinTaskId);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return 0;
}

static UINT32 TestCase(VOID)
{
    UINT32 taskId;
    UINT32 ret;
    UINTPTR temp = 0;
    TskInitParam osTaskInitParam = { 0 };

    g_testCount = 0;

    osTaskInitParam.pfnTaskEntry = (TskEntryFunc)TaskJoinf01;
    osTaskInitParam.stackSize = OS_TSK_TEST_STACK_SIZE;
    osTaskInitParam.pcName = "Join";
    osTaskInitParam.taskPrio = TASK_PRIO_TEST + 1;
    osTaskInitParam.resved = LOS_TASK_ATTR_JOINABLE;

    ret = LOS_TaskCreate(&g_joinTaskId, &osTaskInitParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    osTaskInitParam.pfnTaskEntry = (TskEntryFunc)TaskJoinf02;
    osTaskInitParam.stackSize = OS_TSK_TEST_STACK_SIZE;
    osTaskInitParam.pcName = "deatch";
    osTaskInitParam.taskPrio = TASK_PRIO_TEST - 1;

    ret = LOS_TaskCreate(&taskId, &osTaskInitParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = LOS_TaskJoin(g_joinTaskId, &temp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(temp, taskId, temp);

    return LOS_OK;
}

VOID ItLosTask122(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosTask122", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION);
}


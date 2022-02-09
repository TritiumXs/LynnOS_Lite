/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#include "It_los_queue.h"


static VOID TaskF01(VOID)
{
    UINT32 ret;
    CHAR buff2[QUEUE_SHORT_BUFFER_LENGTH] = "";

    g_testCount++;

    ret = LOS_QueueRead(g_testQueueId01, &buff2, QUEUE_BASE_MSGSIZE, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    g_testCount++;

EXIT:
    LOS_TaskDelete(g_testTaskId02);
}

static VOID TaskF02(VOID)
{
    UINT32 ret;
    CHAR buff1[QUEUE_SHORT_BUFFER_LENGTH] = "UniDSP";

    g_testCount++;

    ret = LOS_QueueCreate("Q1", QUEUE_BASE_NUM, &g_testQueueId01, 0, QUEUE_BASE_MSGSIZE);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_QueueWrite(g_testQueueId01, &buff1, QUEUE_BASE_MSGSIZE, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    g_testCount++;

EXIT:
    LOS_TaskDelete(g_testTaskId01);
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;

    TskInitParam task1 = { 0 };
    TskInitParam task2 = { 0 };
    task1.pfnTaskEntry = (TskEntryFunc)TaskF02;
    task1.pcName = "TskName1";
    task1.stackSize = TASK_STACK_SIZE_TEST;
    // 22, Set the priority according to the task purpose, a smaller number means a higher priority.
    task1.taskPrio = 22;

    g_testCount = 0;

    ret = LOS_TaskCreate(&g_testTaskId01, &task1);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ICUNIT_GOTO_EQUAL(g_testCount, 2, g_testCount, EXIT); // Compare wiht the expected value 2.
    g_testCount++;

    task2.pfnTaskEntry = (TskEntryFunc)TaskF01;
    task2.pcName = "TskName2";
    task2.stackSize = TASK_STACK_SIZE_TEST;
    // 23, Set the priority according to the task purpose, a smaller number means a higher priority.
    task2.taskPrio = 23;

    ret = LOS_TaskCreate(&g_testTaskId02, &task2);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ICUNIT_GOTO_EQUAL(g_testCount, 5, g_testCount, EXIT); // Compare wiht the expected value 5.

    ret = LOS_QueueDelete(g_testQueueId01);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    return LOS_OK;
EXIT:
    LOS_TaskDelete(g_testTaskId01);
    LOS_TaskDelete(g_testTaskId02);
    LOS_QueueDelete(g_testQueueId01);
    return LOS_OK;
}

VOID ItLosQueue042(VOID)
{
    TEST_ADD_CASE("ItLosQueue042", Testcase, TEST_LOS, TEST_QUE, TEST_LEVEL1, TEST_FUNCTION);
}


/*
 * Copyright (c) 2022 Huawei Device Co., Ltd. All rights reserved.
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
#include "It_posix_queue.h"

static VOID *PthreadF01(VOID *argument)
{
    INT32 i;
    struct timespec ts;
    const CHAR *msgptr = MQUEUE_SEND_STRING_TEST;
    UINT32 ret;

    ts.tv_sec = 0xffff;
    ts.tv_nsec = 0;

    g_testCount = 1;

    for (i = 0; i < 5 + 1; i++) { // 5, The loop frequency.
        ret = mq_timedsend(g_gqueue, msgptr, strlen(msgptr), 0, &ts);
        ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);
    }
    ICUNIT_GOTO_EQUAL(g_testCount, 3, g_testCount, EXIT1); // 3, Here, assert the g_testCount.

    g_testCount = 2; // 2, Init test count value.

EXIT1:
    return NULL;
EXIT2:
    g_testCount = 0;
    return NULL;
}

static VOID *PthreadF02(VOID *argument)
{
    INT32 ret;
    CHAR msgrcd[MQUEUE_STANDARD_NAME_LENGTH] = "";

    g_testCount = 3; // 3, Init test count value.

    ret = mq_receive(g_gqueue, msgrcd, MQUEUE_STANDARD_NAME_LENGTH, NULL);
    ICUNIT_GOTO_EQUAL(ret, strlen(MQUEUE_SEND_STRING_TEST), ret, EXIT1);

    TestExtraTaskDelay(2); // 2, Set delay time.
    ret = mq_close(g_gqueue);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = mq_unlink(g_gqname);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    g_testCount = 4; // 4, Init test count value.

    return NULL;
EXIT1:
    mq_close(g_gqueue);
EXIT:
    mq_unlink(g_gqname);
EXIT2:
    g_testCount = 0;
    return NULL;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    struct mq_attr attr = {0};
    pthread_attr_t attr1;
    pthread_t newTh1, newTh2;

    ret = snprintf_s(g_gqname, MQUEUE_STANDARD_NAME_LENGTH, MQUEUE_STANDARD_NAME_LENGTH - 1, \
                     "/mq026_%d", LOS_CurTaskIDGet());
    ICUNIT_GOTO_NOT_EQUAL(ret, -1, ret, EXIT3);

    attr.mq_msgsize = MQUEUE_STANDARD_NAME_LENGTH;
    attr.mq_maxmsg = 5; // 5, queue max message size.
    g_gqueue = mq_open(g_gqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_GOTO_NOT_EQUAL(g_gqueue, (mqd_t)-1, g_gqueue, EXIT);

    g_testCount = 0;

    ret = PosixPthreadInit(&attr1, MQUEUE_PTHREAD_PRIORITY_TEST1);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT1);

    ret = pthread_create(&newTh1, &attr1, PthreadF01, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT1);

    MqueueTaskDelay(5); // 5, Set delay time.
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT1);

    ret = PosixPthreadInit(&attr1, MQUEUE_PTHREAD_PRIORITY_TEST2);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    ret = pthread_create(&newTh2, &attr1, PthreadF02, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    ret = MqueueTaskDelay(5); // 5, Set delay time.
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    ICUNIT_GOTO_EQUAL(g_testCount, 4, g_testCount, EXIT2); // 4, Here, assert the g_testCount.

    ret = PosixPthreadDestroy(&attr1, newTh2);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT1);

    ret = PosixPthreadDestroy(&attr1, newTh1);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    return LOS_OK;

EXIT2:
    PosixPthreadDestroy(&attr1, newTh2);
EXIT1:
    PosixPthreadDestroy(&attr1, newTh1);
EXIT:
    mq_close(g_gqueue);
    mq_unlink(g_gqname);
EXIT3:
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixQueue026
 * @tc.desc: Test interface mq_timedsend
 * @tc.type: FUNC
 * @tc.require: issueI6148G
 */

VOID ItPosixQueue026(VOID)
{
    TEST_ADD_CASE("ItPosixQueue026", Testcase, TEST_POSIX, TEST_QUE, TEST_LEVEL2, TEST_FUNCTION);
}

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

static UINT32 Testcase(VOID)
{
    INT32 i;
    INT32 ret = 0;
    INT32 mqueuePri[MQUEUE_PRIORITY_NUM_TEST] = {MQ_PRIO_MAX, MQ_PRIO_MAX + 1, MQ_PRIO_MAX + 5}; // 5, Mqueue priority.
    CHAR mqname[MQUEUE_STANDARD_NAME_LENGTH] = "";
    const CHAR *msgptr = MQUEUE_SEND_STRING_TEST;
    mqd_t mqueue;

    ret = snprintf_s(mqname, MQUEUE_STANDARD_NAME_LENGTH, MQUEUE_STANDARD_NAME_LENGTH - 1, \
                     "/mq033_%d", LOS_CurTaskIDGet());
    ICUNIT_GOTO_NOT_EQUAL(ret, -1, ret, EXIT2);

    mqueue = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
    ICUNIT_GOTO_NOT_EQUAL(mqueue, (mqd_t)-1, mqueue, EXIT1);

    for (i = 0; i < MQUEUE_PRIORITY_NUM_TEST; i++) {
        ret = mq_send(mqueue, msgptr, strlen(msgptr), mqueuePri[i]);
        ICUNIT_GOTO_EQUAL(ret, -1, ret, EXIT1);
        ICUNIT_GOTO_EQUAL(errno, EINVAL, errno, EXIT1);
    }

    ret = mq_close(mqueue);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = mq_unlink(mqname);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    return LOS_OK;
EXIT1:
    mq_close(mqueue);
EXIT:
    mq_unlink(mqname);
EXIT2:
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixQueue033
 * @tc.desc: Test interface mq_send
 * @tc.type: FUNC
 * @tc.require: issueI6148G
 */

VOID ItPosixQueue033(VOID)
{
    TEST_ADD_CASE("ItPosixQueue033", Testcase, TEST_POSIX, TEST_QUE, TEST_LEVEL2, TEST_FUNCTION);
}

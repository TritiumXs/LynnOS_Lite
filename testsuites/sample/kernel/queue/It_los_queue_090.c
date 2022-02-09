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


static UINT32 Testcase(VOID)
{
    UINT32 ret, i, j;
    UINT32 index;
    UINT32 queueId[LOSCFG_BASE_IPC_QUEUE_LIMIT + 1];
    CHAR filebuf[260] = "abcdeabcde0123456789abcedfghij9876550210abcdeabcde0123456789abcedfghij9876550210abcdeabcde0123"
                        "456789abcedfghij9876550210abcdeabcde0123456789abcedfghij9876550210abcdeabcde0123456789abcedfgh"
                        "ij9876550210abcdeabcde0123456789abcedfghij9876550210lalalalalalalala";
    CHAR readbuf[260] = ""; // 260, buffersize
    QueueInfo queueInfo;

    const UINT32 len = 1;
    const UINT32 count = 256; // 256, set maxMsgSize

    UINT32 limit = LOSCFG_BASE_IPC_QUEUE_LIMIT - QUEUE_EXISTED_NUM;
    for (index = 0; index < limit; index++) {
        ret = LOS_QueueCreate(NULL, len, &queueId[index], 0, count);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

        ret = LOS_QueueInfoGet(queueId[index], &queueInfo);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
        ICUNIT_GOTO_EQUAL(queueInfo.queueLen, len, queueInfo.queueLen, EXIT);
        ICUNIT_GOTO_EQUAL(queueInfo.queueId, queueId[index], queueInfo.queueId, EXIT);
    }

    ret = LOS_QueueCreate("Q1", len, &queueId[LOSCFG_BASE_IPC_QUEUE_LIMIT + 1], 0, count);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_QUEUE_CB_UNAVAILABLE, ret, EXIT);

    for (j = 0; j < 100; j++) { // 100, test times
        for (index = 0; index < limit; index++) {
            for (i = 0; i < len; i++) {
                ret = LOS_QueueWrite(queueId[index], filebuf, count, 0);
                ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
            }
            ret = LOS_QueueWrite(queueId[index], filebuf, count, 0);
            ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_QUEUE_ISFULL, ret, EXIT);

            for (i = 0; i < len; i++) {
                (void)memset_s(readbuf, sizeof(readbuf), 0, 260); // 260, buffersize
                ret = LOS_QueueRead(queueId[index], readbuf, count, 0);
                ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
            }
            ret = LOS_QueueRead(queueId[index], readbuf, count, 0);
            ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_QUEUE_ISEMPTY, ret, EXIT);
        }
    }
    ret = LOS_QueueWrite(queueId[limit - 1], filebuf, count, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_QueueRead(queueId[limit - 1], readbuf, count, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_QueueInfoGet(queueId[limit - 1], &queueInfo);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(queueInfo.queueLen, len, queueInfo.queueLen, EXIT);
    ICUNIT_GOTO_EQUAL(queueInfo.queueId, queueId[limit - 1], queueInfo.queueId, EXIT);

    ret = LOS_QueueRead(queueId[limit - 1], readbuf, count, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_QUEUE_ISEMPTY, ret, EXIT);

    for (index = 0; index < limit; index++) {
        ret = LOS_QueueDelete(queueId[index]);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }
    return LOS_OK;

EXIT:
    for (index = 0; index < limit; index++) {
        LOS_QueueDelete(queueId[index]);
    }
    return LOS_OK;
}

VOID ItLosQueue090(VOID)
{
    TEST_ADD_CASE("ItLosQueue090", Testcase, TEST_LOS, TEST_QUE, TEST_LEVEL0, TEST_FUNCTION);
}


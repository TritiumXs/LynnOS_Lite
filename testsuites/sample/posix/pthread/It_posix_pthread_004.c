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

#include "It_posix_pthread.h"

static UINT32 Testcase(VOID)
{
    pthread_attr_t attr;
    UINT32 ret;
    int detachstate;

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setdetachstate(NULL, PTHREAD_CREATE_DETACHED);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE - 2); // 2: used to create a invalid argument
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_attr_setdetachstate(&attr, 3); // 3, test the param of function.
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_attr_getdetachstate(NULL, &detachstate);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_attr_getdetachstate(&attr, NULL);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_getdetachstate(&attr, &detachstate);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(detachstate, PTHREAD_CREATE_DETACHED, detachstate, EXIT);

EXIT:
    ret = pthread_attr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

/**
 * @tc.name: ItPosixPthread004
 * @tc.desc: Test interface pthread_setdetachstate
 * @tc.type: FUNC
 * @tc.require: issueI5TIRQ
 */

VOID ItPosixPthread004(VOID)
{
    TEST_ADD_CASE("ItPosixPthread004", Testcase, TEST_POSIX, TEST_PTHREAD, TEST_LEVEL2, TEST_FUNCTION);
}

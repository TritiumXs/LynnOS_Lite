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

#include "It_posix_mutex.h"


static UINT32 Testcase(VOID)
{
    int ret;
    pthread_mutex_t mutex1084 = TEST_MUTEX_INIT;
    pthread_mutex_t mutex2084 = TEST_MUTEX_INIT;

    ret = pthread_mutex_init(&mutex1084, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_trylock(&mutex1084);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_init(&mutex2084, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_lock(&mutex2084);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_trylock(&mutex2084);
    ICUNIT_ASSERT_EQUAL(ret, EBUSY, ret);

    ret = pthread_mutex_unlock(&mutex1084);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_destroy(&mutex1084);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_unlock(&mutex2084);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_destroy(&mutex2084);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return LOS_OK;
}

/**
 * @tc.name: ItPosixMux001
 * @tc.desc: Test interface pthread_mutex_trylock
 * @tc.type: FUNC
 * @tc.require: issueI5WZI6
 */

VOID ItPosixMux001(void)
{
    TEST_ADD_CASE("ItPosixMux001", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}

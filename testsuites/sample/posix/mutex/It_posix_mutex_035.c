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

static pthread_mutex_t g_mutex077;

static VOID *TaskF01(void *arg)
{
    UINT32 ret;
    ret = pthread_mutex_trylock(&g_mutex077);
    ICUNIT_TRACK_EQUAL(ret, EBUSY, ret);

    g_testCount++;

    ret = pthread_mutex_lock(&g_mutex077);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);

    ret = pthread_mutex_unlock(&g_mutex077);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);
    g_testCount++;

    return NULL;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    pthread_t newTh;
    pthread_attr_t attr;
    pthread_mutexattr_t mutexAttr;

    g_testCount = 0;
    ret = pthread_mutex_lock(NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_mutex_lock(&g_mutex077);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_mutex_unlock(&g_mutex077);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);
    pthread_mutexattr_init(&mutexAttr);

    pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_ERRORCHECK);

    ret = pthread_mutex_init(&g_mutex077, &mutexAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_lock(&g_mutex077);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = PosixPthreadInit(&attr, 4); // 4, Set thread priority.
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = pthread_create(&newTh, &attr, TaskF01, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    LOS_TaskDelay(5); // 5, set delay time.
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    ret = pthread_mutex_unlock(&g_mutex077);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    TestExtraTaskDelay(1);
    ICUNIT_ASSERT_EQUAL(g_testCount, 2, g_testCount); // 2, Assert that g_testCount.

    ret = pthread_mutex_lock(&g_mutex077);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_lock(&g_mutex077);
    ICUNIT_ASSERT_EQUAL(ret, EDEADLK, ret);

    ret = pthread_mutex_lock(&g_mutex077);
    ICUNIT_ASSERT_EQUAL(ret, EDEADLK, ret);

    ret = pthread_mutex_unlock(&g_mutex077);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_unlock(&g_mutex077);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_unlock(&g_mutex077);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_destroy(&g_mutex077);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

/**
 * @tc.name: ItPosixMux035
 * @tc.desc: Test interface pthread_mutexattr_settype
 * @tc.type: FUNC
 * @tc.require: issueI5WZI6
 */

VOID ItPosixMux035(void)
{
    TEST_ADD_CASE("ItPosixMux035", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}

/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <unistd.h>
#include <stdio.h>
#include <securec.h>
#include <pthread.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/ipc.h>
#include <limits.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "ohos_types.h"
#include "hctest.h"
#include "utils.h"
#include "PthreadTest.h"
#include "mt_utils.h"
#include "log.h"

LITE_TEST_SUIT(PROCESS, PthreadBasicApi, PthreadBasicApiTestSuite);

static BOOL PthreadBasicApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL PthreadBasicApiTestSuiteTearDown(void)
{
    LOG("+-------------------------------------------+\n");
    return TRUE;
}

void *ThreadPthreadCreateBasic(void *arg)
{
    char *s = (char*)arg;
    EXPECT_STREQ(s, "1234567890 !@#$%^&*()_= ZXCVBNM [];'./>?:\" +-*/qwertyuiopasdfghjklzxcvbnm");
    return arg;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_CREATE_0100
 * @tc.name     pthread_create create a thread
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testPthreadCreateBasic, Function | MediumTest | Level2)
{
    pthread_t tid;
    char str[] = "1234567890 !@#$%^&*()_= ZXCVBNM [];'./>?:\" +-*/qwertyuiopasdfghjklzxcvbnm";
    ASSERT_EQ(pthread_create(&tid, NULL, ThreadPthreadCreateBasic, (void*)str), 0);

    EXPECT_EQ(pthread_join(tid, NULL), 0);
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_JOIN_0200
 * @tc.name     Test the function of pthread_join to get the return value
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testJoinReturn, Function | MediumTest | Level3)
{
    pthread_t tid;
    int num = 4;
    void *joinRe = NULL;

    ASSERT_EQ(pthread_create(&tid, NULL, ThreadPublic, (void*)&num), 0);
    EXPECT_EQ(pthread_join(tid, &joinRe), 0);

    int *p = (int*)joinRe;
    EXPECT_TRUE(&num == p);
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_JOIN_0300
 * @tc.name     Test the function about pthread_join, but child thread Exited
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testJoinExited, Function | MediumTest | Level3)
{
    pthread_t tid;
    ASSERT_EQ(pthread_create(&tid, NULL, ThreadPublic, NULL), 0);
    Msleep(50);
    EXPECT_EQ(pthread_join(tid, NULL), 0);
}

void *ThreadPthreadExitThread(void *arg)
{
    pthread_exit(arg);
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_EXIT_0100
 * @tc.name     Test the return function of pthread_exit in the child thread
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testPthreadExitThread, Function | MediumTest | Level3)
{
    pthread_t tid;
    int num = 4;
    void *joinRe = NULL;

    ASSERT_EQ(pthread_create(&tid, NULL, ThreadPthreadExitThread, (void*)&num), 0);
    EXPECT_EQ(pthread_join(tid, &joinRe), 0);
    int *p = (int*)joinRe;
    EXPECT_TRUE(&num == p);
}

void FunPthreadExit(void *arg)
{
    pthread_exit(arg);
}

void *ThreadPthreadExitFunction(void *arg)
{
    FunPthreadExit(arg);
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_EXIT_0200
 * @tc.name     Test the return function of pthread_exit in the child thread function
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testPthreadExitFunction, Function | MediumTest | Level3)
{
    pthread_t tid;
    int num = 4;
    void *joinRe = NULL;

    ASSERT_EQ(pthread_create(&tid, NULL, ThreadPthreadExitFunction, (void*)&num), 0);
    EXPECT_EQ(pthread_join(tid, &joinRe), 0);
    int *p = (int*)joinRe;
    EXPECT_TRUE(&num == p);
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_DETACH_0100
 * @tc.name     Use pthread_detach to detach child threads
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testPthreadDetach, Function | MediumTest | Level3)
{
    pthread_t tid;
    ASSERT_EQ(pthread_create(&tid, NULL, ThreadPublic, NULL), 0);
    EXPECT_EQ(pthread_detach(tid), 0);
    EXPECT_EQ(pthread_join(tid, NULL), EINVAL);
}

void *ThreadPthreadEqual(void *arg)
{
    pthread_t *tid = (pthread_t*)arg;
    EXPECT_NE(pthread_equal(*tid, pthread_self()), 0);
    Msleep(10);
    return arg;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_EQUAL_0100
 * @tc.name     Use pthread_equal checks process equality
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testPthreadEqual, Function | MediumTest | Level3)
{
    pthread_t tid;
    ASSERT_EQ(pthread_create(&tid, NULL, ThreadPthreadEqual, (void*)&tid), 0);
    EXPECT_EQ(pthread_equal(tid, pthread_self()), 0);
    EXPECT_EQ(pthread_join(tid, NULL), 0);
}

RUN_TEST_SUITE(PthreadBasicApiTestSuite);


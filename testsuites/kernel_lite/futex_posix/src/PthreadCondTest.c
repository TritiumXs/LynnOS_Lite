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
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include "utils.h"
#include "log.h"
#include "KernelConstants.h"
#include "FutexTest.h"
#include "mt_utils.h"

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(FUTEX, PthreadCondApiTest, PthreadCondApiTestSuite);

static BOOL PthreadCondApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL PthreadCondApiTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_COND_INIT_0100
 * @tc.name     pthread_cond_init initializes condition variables
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadCondApiTestSuite, testPthreadCondInit, Function | MediumTest | Level2)
{
    pthread_condattr_t condattr;
    TEST_ASSERT_EQUAL_INT(0, pthread_condattr_init(&condattr));
    pthread_cond_t cond1;
    TEST_ASSERT_EQUAL_INT(0, pthread_cond_init(&cond1, &condattr));

    pthread_cond_t cond2;
    TEST_ASSERT_EQUAL_INT(0, pthread_cond_init(&cond2, NULL));
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_COND_DESTROY_0100
 * @tc.name     pthread_cond_destroy destroy condition variables
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadCondApiTestSuite, testPthreadCondDestroy, Function | MediumTest | Level3)
{
    pthread_condattr_t condattr;
    TEST_ASSERT_EQUAL_INT(0, pthread_condattr_init(&condattr));
    pthread_cond_t cond1;
    TEST_ASSERT_EQUAL_INT(0, pthread_cond_init(&cond1, &condattr));
    TEST_ASSERT_EQUAL_INT(0, pthread_cond_destroy(&cond1));

    pthread_cond_t cond2;
    TEST_ASSERT_EQUAL_INT(0, pthread_cond_init(&cond2, NULL));
    TEST_ASSERT_EQUAL_INT(0, pthread_cond_destroy(&cond2));

    pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
    TEST_ASSERT_EQUAL_INT(0, pthread_cond_destroy(&cond3));
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_CONDATTR_INIT_0100
 * @tc.name     Init and destroy operations
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadCondApiTestSuite, testPthreadCondattrInit, Function | MediumTest | Level2)
{
    pthread_condattr_t condattr;

    TEST_ASSERT_EQUAL_INT(0, pthread_condattr_init(&condattr));
    TEST_ASSERT_EQUAL_INT(0, pthread_condattr_destroy(&condattr));
    TEST_ASSERT_EQUAL_INT(0, pthread_condattr_init(&condattr));
}

pthread_mutex_t g_mtx3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond3 = PTHREAD_COND_INITIALIZER;

// pthread_cond_broadcast
void *ThreadPthreadCondBroadcast1(void *arg)
{
    int *testIntP = (int *)arg;
    Msleep(20);
    TEST_ASSERT_EQUAL_INT(pthread_mutex_lock(&g_mtx3), 0);
    *testIntP = 10;
    TEST_ASSERT_EQUAL_INT(pthread_cond_broadcast(&g_cond3), 0);
    TEST_ASSERT_EQUAL_INT(pthread_mutex_unlock(&g_mtx3), 0);
    return arg;
}

// pthread_cond_wait
void *ThreadPthreadCondBroadcast2(void *arg)
{
    int *testIntP = (int *)arg;
    TEST_ASSERT_EQUAL_INT(0, pthread_mutex_lock(&g_mtx3));
    TEST_ASSERT_EQUAL_INT(0, pthread_cond_wait(&g_cond3, &g_mtx3));
    (*testIntP)++;
    TEST_ASSERT_EQUAL_INT(0, pthread_mutex_unlock(&g_mtx3));
    return arg;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_COND_BROADCAST_0100
 * @tc.name     Use pthread_cond_broadcast to release conditional semaphore
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadCondApiTestSuite, testPthreadCondBroadcast, Function | MediumTest | Level3)
{
    pthread_t tid[3];
    int testInt = 0;

    TEST_ASSERT_EQUAL_INT(0, pthread_create(&tid[0], NULL, ThreadPthreadCondBroadcast1, (void*)&testInt));
    TEST_ASSERT_EQUAL_INT(0, pthread_create(&tid[1], NULL, ThreadPthreadCondBroadcast2, (void*)&testInt));
    TEST_ASSERT_EQUAL_INT(0, pthread_create(&tid[2], NULL, ThreadPthreadCondBroadcast2, (void*)&testInt));

    Msleep(100);
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);
    TEST_ASSERT_EQUAL_INT(0, pthread_cond_destroy(&g_cond3));
    TEST_ASSERT_EQUAL_INT(0, pthread_mutex_destroy(&g_mtx3));
    TEST_ASSERT_EQUAL_INT(12, testInt);
}

RUN_TEST_SUITE(PthreadCondApiTestSuite);

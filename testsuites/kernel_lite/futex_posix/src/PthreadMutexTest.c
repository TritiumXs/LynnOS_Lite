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
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include "utils.h"
#include "mt_utils.h"
#include "log.h"
#include "FutexTest.h"

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(FUTEX, PthreadMutexApiTest, PthreadMutexApiTestSuite);

static BOOL PthreadMutexApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL PthreadMutexApiTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/********************************************* Test case dividing line ***********************************************/

struct PthreadMutexCond {
    int loopNum;
    int countMax;
    int count;
    int top;
    int bottom;
    pthread_cond_t notfull;
    pthread_cond_t notempty;
    pthread_mutex_t mutex;
};

struct PthreadMutexCond g_st1;
void g_st1Init(void)
{
    memset(&g_st1, 0, sizeof(struct PthreadMutexCond));
    g_st1.loopNum = 10;
    g_st1.countMax = 5;
    g_st1.count = 0;
    g_st1.top = 0;
    g_st1.bottom = 0;
    g_st1.notfull = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    g_st1.notempty = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    g_st1.mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
}

void *PthreadProduce(void *arg)
{
    for (int i = 0; i < g_st1.loopNum; i++) {
        TEST_ASSERT_EQUAL_INT(pthread_mutex_lock(&g_st1.mutex), 0);
        // check full
        if ((g_st1.top + 1) % g_st1.countMax == g_st1.bottom) {
            TEST_ASSERT_EQUAL_INT(pthread_cond_wait(&g_st1.notempty, &g_st1.mutex), 0);
        }
        // Produce
        g_st1.top = (g_st1.top + 1) % g_st1.countMax;
        g_st1.count++;
        printf("producer g_st1.top = %d", g_st1.top);

        TEST_ASSERT_EQUAL_INT(pthread_cond_signal(&g_st1.notempty), 0);
        TEST_ASSERT_EQUAL_INT(pthread_mutex_unlock(&g_st1.mutex), 0);
        Msleep(10);
    }
    return arg;
}

void *PthreadConsume(void *arg)
{
    for (int i = 0; i < g_st1.loopNum; i++) {
        TEST_ASSERT_EQUAL_INT(pthread_mutex_lock(&g_st1.mutex), 0);
        // check empty
        if (g_st1.top == g_st1.bottom) {
            TEST_ASSERT_EQUAL_INT(pthread_cond_wait(&g_st1.notempty, &g_st1.mutex), 0);
        }
        // Consume
        g_st1.bottom = (g_st1.bottom + 1) % g_st1.countMax;
        g_st1.count--;
        LOG("consume g_st1.bottom = %d", g_st1.bottom);

        TEST_ASSERT_EQUAL_INT(pthread_cond_signal(&g_st1.notempty), 0);
        TEST_ASSERT_EQUAL_INT(pthread_mutex_unlock(&g_st1.mutex), 0);

        Msleep(10);
    }
    return arg;
}

/**
 * @tc.number     SUB_KERNEL_FUTEX_MUTEX_ALL_0300
 * @tc.name       test pthread_mutex with condition variable, produce and consume
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadMutexApiTestSuite, testPthreadMutexCond, Function | MediumTest | Level3)
{
    pthread_t tid[2];

    // g_st1Init();
    g_st1.count = 0;

    memset(tid, 0, sizeof(tid));
    TEST_ASSERT_EQUAL_INT(pthread_create(&tid[0], NULL, PthreadProduce, NULL), 0);
    TEST_ASSERT_EQUAL_INT(pthread_create(&tid[1], NULL, PthreadConsume, NULL), 0);
    TEST_ASSERT_EQUAL_INT(pthread_join(tid[0], NULL), 0);
    TEST_ASSERT_EQUAL_INT(pthread_join(tid[1], NULL), 0);
    TEST_ASSERT_EQUAL_INT(g_st1.count, 0);
}

RUN_TEST_SUITE(PthreadMutexApiTestSuite);

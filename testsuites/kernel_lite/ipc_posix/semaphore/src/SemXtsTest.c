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
#include <semaphore.h>
#include <pthread.h>
#include "utils.h"
#include "log.h"

#include "ohos_types.h"
#include "hctest.h"
#include "SemTest.h"

#define KERNEL_NS_PER_SECOND 1000000000
#define KERNEL_100MS_BY_NS   100000000

LITE_TEST_SUIT(IPC, SemApi, IpcSemApiXtsTestSuite);

static BOOL IpcSemApiXtsTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IpcSemApiXtsTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0100
 * @tc.name     Use sem_init initialize the semaphore with 0
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemInitA, Function | MediumTest | Level2)
{
    sem_t sem;
    int semValue = 0;
    int testValue = 0;

    EXPECT_NE(sem_init(&sem, 0, testValue), -1);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_TRUE(semValue == testValue);
    EXPECT_EQ(sem_destroy(&sem), 0);
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0110
 * @tc.name     Use sem_init initialize the semaphore with 1
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemInitB, Function | MediumTest | Level2)
{
    sem_t sem;
    int semValue = 0;
    int testValue = 1;

    EXPECT_NE(sem_init(&sem, 0, testValue), -1);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, testValue);
    EXPECT_EQ(sem_destroy(&sem), 0);
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0120
 * @tc.name     Use sem_init initialize the semaphore with 100
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemInitC, Function | MediumTest | Level2)
{
    sem_t sem;
    int semValue = 0;
    int testValue = 10;

    EXPECT_NE(sem_init(&sem, 0, testValue), -1);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, testValue);
    EXPECT_EQ(sem_destroy(&sem), 0);
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_POST_0100
 * @tc.name     sem_post increases the semaphore count
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemPost, Function | MediumTest | Level2)
{
    sem_t sem;
    int semValue = 0;

    ASSERT_NE(sem_init(&sem, 0, 0), -1);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);

    EXPECT_EQ(sem_post(&sem), 0);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 1);

    EXPECT_EQ(sem_post(&sem), 0);
    EXPECT_EQ(sem_post(&sem), 0);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 3);

    EXPECT_EQ(sem_destroy(&sem), 0);
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_WAIT_0100
 * @tc.name     sem_wait get semaphore
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemWait, Function | MediumTest | Level2)
{
    sem_t sem;
    int semValue = 0;

    ASSERT_NE(sem_init(&sem, 0, 3), -1);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 3);

    EXPECT_EQ(sem_wait(&sem), 0);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 2);

    EXPECT_EQ(sem_wait(&sem), 0);
    EXPECT_EQ(sem_wait(&sem), 0);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);

    EXPECT_EQ(sem_destroy(&sem), 0);
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_DESTROY_0100
 * @tc.name     check sem_destroy function
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemdestroy, Function | MediumTest | Level3)
{
    sem_t sem;
    ASSERT_EQ(sem_init(&sem, 0, 0), 0);
    EXPECT_EQ(sem_destroy(&sem), 0);
}

void *ThreadChat_F01(void *arg)
{
    sem_t *sem = (sem_t*)arg;
    int semValue = 0;

    EXPECT_EQ(sem_getvalue(sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);
    EXPECT_EQ(sem_wait(sem), 0);
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_CHAT_0100
 * @tc.name     Inter-thread communication, check sem_wait function
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testThreadChat, Function | MediumTest | Level3)
{
    pthread_t tid;
    sem_t sem;
    int reInt = 0;
    int semValue = 0;

    ASSERT_EQ(sem_init(&sem, 0, 0), 0);

    reInt = pthread_create(&tid, NULL, ThreadChat_F01, (void*)&sem);
    ASSERT_EQ(reInt, 0);

    Msleep(20);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);

    EXPECT_EQ(sem_post(&sem), 0);
    EXPECT_EQ(sem_post(&sem), 0);

    Msleep(20);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 1);

    reInt = pthread_join(tid, NULL);
    EXPECT_EQ(reInt, 0);
    EXPECT_EQ(sem_destroy(&sem), 0);
}

void *ThreadSemTimedWait(void *arg)
{
    struct timespec ts = {0};
    sem_t *sem = (sem_t*)arg;
    int semValue = 0;

    EXPECT_EQ(sem_getvalue(sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec = ts.tv_sec + (ts.tv_nsec + KERNEL_100MS_BY_NS) / KERNEL_NS_PER_SECOND;
    ts.tv_nsec = (ts.tv_nsec + KERNEL_100MS_BY_NS) % KERNEL_NS_PER_SECOND;
    Msleep(50);
    EXPECT_EQ(sem_timedwait(sem, &ts), 0);
    EXPECT_EQ(sem_getvalue(sem, &semValue), 0);

    Msleep(100);
    EXPECT_EQ(semValue, 1);
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_CHAT_0300
 * @tc.name     Inter-thread communication, check sem_timedwait function
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testThreadSemTimedWait, Function | MediumTest | Level3)
{
    pthread_t tid;
    sem_t sem;
    int reInt = 0;
    int semValue = 0;

    ASSERT_EQ(sem_init(&sem, 0, 0), 0);

    reInt = pthread_create(&tid, NULL, ThreadSemTimedWait, (void*)&sem);
    ASSERT_EQ(reInt, 0);

    Msleep(10);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);
    EXPECT_EQ(sem_post(&sem), 0);
    EXPECT_EQ(sem_post(&sem), 0);

    reInt = pthread_join(tid, NULL);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 1);
    EXPECT_EQ(reInt, 0);
    EXPECT_EQ(sem_destroy(&sem), 0);
}
 
void *ThreadNThreadWait_F01(void *arg)
{
    sem_t *sem = (sem_t*)arg;
    int semValue = 0;

    Msleep(100);
    EXPECT_EQ(sem_getvalue(sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);
    EXPECT_EQ(sem_wait(sem), 0);
    return NULL;
}

void *ThreadNThreadWait_F02(void *arg)
{
    sem_t *sem = (sem_t*)arg;
    int semValue = 0;
    EXPECT_EQ(sem_getvalue(sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);

    Msleep(300);
    EXPECT_EQ(sem_getvalue(sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);

    Msleep(200);
    EXPECT_EQ(sem_getvalue(sem, &semValue), 0);
    EXPECT_EQ(semValue, 1);
    EXPECT_EQ(sem_wait(sem), 0);
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_CHAT_0400
 * @tc.name     N threads wait, main thread post
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testNThreadWait, Function | MediumTest | Level4)
{
    pthread_t tid1;
    pthread_t tid2;
    sem_t sem;
    int reInt = 0;
    int semValue = 0;

    ASSERT_EQ(sem_init(&sem, 0, 0), 0);

    reInt = pthread_create(&tid1, NULL, ThreadNThreadWait_F01, (void*)&sem);
    ASSERT_EQ(reInt, 0);

    reInt = pthread_create(&tid2, NULL, ThreadNThreadWait_F02, (void*)&sem);
    ASSERT_EQ(reInt, 0);

    Msleep(200);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);
    EXPECT_EQ(sem_post(&sem), 0);

    Msleep(20);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);

    Msleep(200);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 0);
    EXPECT_EQ(sem_post(&sem), 0);

    Msleep(20);
    EXPECT_EQ(sem_getvalue(&sem, &semValue), 0);
    EXPECT_EQ(semValue, 1);

    reInt = pthread_join(tid1, NULL);
    EXPECT_EQ(reInt, 0);
    reInt = pthread_join(tid2, NULL);
    EXPECT_EQ(reInt, 0);
    EXPECT_EQ(sem_destroy(&sem), 0);
}

RUN_TEST_SUITE(IpcSemApiXtsTestSuite);

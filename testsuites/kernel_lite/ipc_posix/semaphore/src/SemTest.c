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
#include <semaphore.h>
#include <sys/resource.h>
#include <limits.h>
#include "utils.h"
#include "SemTest.h"
#include "ohos_types.h"
#include "hctest.h"

#define SEM_VALUE_MAX 0xFFFE

static int g_semTestStep = 0;

LITE_TEST_SUIT(IPC, SemApi, IpcSemApiTestSuite);

static BOOL IpcSemApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IpcSemApiTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemInit0100, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;
    int testValue[3] = {0, 1, 10};

    for (int i = 0; i < (int)(sizeof(testValue)/sizeof(int)); i++) {
        printf("test index of testValue %d\n", i);

        ret = sem_init((sem_t *)&sem, 0, testValue[0]);
        TEST_ASSERT_EQUAL_INT(ret, 0);

        ret = sem_destroy(&sem);
        TEST_ASSERT_EQUAL_INT(ret, 0);
    }
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemPost0100, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;

    ret = sem_init((sem_t *)&sem, 0, 0);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_post(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_post(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_post(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_destroy(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemWait0100, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;

    ret = sem_init((sem_t *)&sem, 0, 3);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_wait(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_wait(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_wait(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_destroy(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

void *ThreadChat(void *arg)
{
    int ret;
    sem_t *sem = (sem_t *)arg;

    TEST_ASSERT_EQUAL_INT(g_semTestStep, 0);

    g_semTestStep = 1;
    ret = sem_wait(sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    g_semTestStep = 2;
    return NULL;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testThreadChat0100, Function | MediumTest | Level3)
{
    pthread_t tid;
    sem_t sem;
    int ret = 0;
    struct timespec req;
    g_semTestStep = 0;

    ret = sem_init((sem_t *)&sem, 0, 0);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = pthread_create(&tid, NULL, ThreadChat, (void *)&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    req.tv_sec = 0;
    req.tv_nsec = TEN_CONT * NANO_MS;
    nanosleep(&req, NULL);
    TEST_ASSERT_EQUAL_INT(g_semTestStep, 1);

    ret = sem_post(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    nanosleep(&req, NULL);
    TEST_ASSERT_EQUAL_INT(g_semTestStep, 2);

    ret = sem_post(&sem);
    TEST_ASSERT_EQUAL_INT(g_semTestStep, 2);

    ret = pthread_join(tid, NULL);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_destroy(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

void *ThreadNThreadWait1(void *arg)
{
    int ret;
    sem_t *sem = (sem_t *)arg;
    struct timespec req;

    req.tv_sec = 0;
    req.tv_nsec = HUNDRED_CONT * NANO_MS;

    nanosleep(&req, NULL);
    g_semTestStep = 1;
    ret = sem_wait(sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    g_semTestStep = 2;
    return NULL;
}

void *ThreadNThreadWait2(void *arg)
{
    int ret;
    sem_t *sem = (sem_t *)arg;
    struct timespec req;

    req.tv_sec = 0;
    req.tv_nsec = 300 * NANO_MS;
    nanosleep(&req, NULL);
    g_semTestStep = 3;

    req.tv_nsec = 200 * NANO_MS;
    nanosleep(&req, NULL);
    ret = sem_wait(sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    g_semTestStep = 4;
    return NULL;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testThreadChat0400, Function | MediumTest | Level4)
{
    pthread_t tid1;
    pthread_t tid2;
    sem_t sem;
    int ret = 0;
    struct timespec req;

    req.tv_sec = 0;
    req.tv_nsec = 200 * NANO_MS;
    g_semTestStep = 0;

    ret = sem_init((sem_t *)&sem, 0, 0);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = pthread_create(&tid1, NULL, ThreadNThreadWait1, (void *)&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = pthread_create(&tid2, NULL, ThreadNThreadWait2, (void *)&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    nanosleep(&req, NULL);
    TEST_ASSERT_EQUAL_INT(g_semTestStep, 1);

    ret = sem_post(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    req.tv_nsec = 20 * NANO_MS;
    nanosleep(&req, NULL);
    TEST_ASSERT_EQUAL_INT(g_semTestStep, 2);

    req.tv_nsec = 200 * NANO_MS;
    nanosleep(&req, NULL);
    TEST_ASSERT_EQUAL_INT(g_semTestStep, 3);

    ret = sem_post(&sem);
    req.tv_nsec = 20 * NANO_MS;
    nanosleep(&req, NULL);
    TEST_ASSERT_EQUAL_INT(g_semTestStep, 3);

    ret = pthread_join(tid1, NULL);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = pthread_join(tid2, NULL);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    TEST_ASSERT_EQUAL_INT(g_semTestStep, 4);

    ret = sem_destroy(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemInitAbnormal0200, Function | MediumTest | Level3)
{
    int ret;
    sem_t sem;
    unsigned int gtSemMax = (unsigned int)SEM_VALUE_MAX + 1;
    printf("> SEM_VALUE_MAX = %d \n", SEM_VALUE_MAX);

    ret = sem_init(&sem, 0, SEM_VALUE_MAX);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_destroy(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_init(&sem, 0, gtSemMax);
    TEST_ASSERT_EQUAL_INT(ret, -1);

    TEST_ASSERT_EQUAL_INT(errno, EINVAL);

    ret = sem_init(&sem, 0, 1);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_destroy(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemPostAbnormal, Function | MediumTest | Level3)
{
    int ret;
    sem_t sem;

    ret = sem_init(&sem, 0, SEM_VALUE_MAX);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_post(&sem);
    // not support OVERFLOW
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = sem_destroy(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemTimedWaitAbnormalA, Function | MediumTest | Level3)
{
    int ret;
    struct timespec ts;
    sem_t sem;

    ret = sem_init(&sem, 0, 0);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ts.tv_sec = 0;
    ts.tv_nsec = -2;
    ret = sem_timedwait(&sem, &ts);
    TEST_ASSERT_EQUAL_INT(ret, -1);

    TEST_ASSERT_EQUAL_INT(errno, EINVAL);

    ret = sem_destroy(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemTimedWaitAbnormalB, Function | MediumTest | Level3)
{
    int ret;
    struct timespec ts;
    sem_t sem;

    ret = sem_init(&sem, 0, 0);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ts.tv_sec = time(NULL);
    ts.tv_nsec = NANO_S;
    ret = sem_timedwait(&sem, &ts);
    TEST_ASSERT_EQUAL_INT(ret, -1);

    TEST_ASSERT_EQUAL_INT(errno, EINVAL);

    ret = sem_destroy(&sem);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

RUN_TEST_SUITE(IpcSemApiTestSuite);

void IpcSemApiTest()
{
    printf("begin IpcSemApiTest....\n");

    RUN_ONE_TESTCASE(testSemInit0100);
    RUN_ONE_TESTCASE(testSemPost0100);
    RUN_ONE_TESTCASE(testSemWait0100);
    RUN_ONE_TESTCASE(testThreadChat0100);
    RUN_ONE_TESTCASE(testThreadChat0400);
    RUN_ONE_TESTCASE(testSemInitAbnormal0200);
    RUN_ONE_TESTCASE(testSemPostAbnormal);
    RUN_ONE_TESTCASE(testSemTimedWaitAbnormalA);
    RUN_ONE_TESTCASE(testSemTimedWaitAbnormalB);

    return;
}

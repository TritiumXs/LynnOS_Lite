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
#define SEM_VALUE_MAX        0xFFFE

LITE_TEST_SUIT(IPC, SemAbnormalTestSuite, SemAbnormalTestSuite);

static BOOL SemAbnormalTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL SemAbnormalTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0200
 * @tc.name     Use sem_init initialized value when value is SEM_VALUE_MAX
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemInitAbnormalSemvaluemax, Function | MediumTest | Level3)
{
    sem_t sem;
    int semValue = 0;
    LOG("> SEM_VALUE_MAX = %d", SEM_VALUE_MAX);

    TEST_ASSERT_NOT_EQUAL(-1, sem_init(&sem, 0, SEM_VALUE_MAX));
    TEST_ASSERT_EQUAL_INT(0, sem_getvalue(&sem, &semValue));
    TEST_ASSERT_EQUAL_INT(SEM_VALUE_MAX, semValue);
    TEST_ASSERT_EQUAL_INT(0, sem_destroy(&sem));
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0210
 * @tc.name     Use sem_init initialized value when value is greater than SEM_VALUE_MAX
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemInitAbnormalGtsemvaluemax, Function | MediumTest | Level3)
{
    sem_t sem;
    unsigned int gtSemMax = (unsigned int)SEM_VALUE_MAX + 1;
    LOG("> SEM_VALUE_MAX = %d", SEM_VALUE_MAX);

    if (sem_init(&sem, 0, gtSemMax) == -1) {
        TEST_ASSERT_EQUAL_INT(errno, EINVAL);
    } else {
        LOG("> sem_init return unexpected");
        ADD_FAILURE();
    }
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0220
 * @tc.name     Use sem_init initialized value twice
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemInitAbnormalInitTwice, Function | MediumTest | Level3)
{
    sem_t sem;
    TEST_ASSERT_NOT_EQUAL(-1, sem_init(&sem, 0, 1));
    TEST_ASSERT_NOT_EQUAL(-1, sem_init(&sem, 0, 1));
    TEST_ASSERT_EQUAL_INT(0, sem_destroy(&sem));
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_POST_0200
 * @tc.name     sem_post increases the semaphore count near the maximum value
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemPostAbnormal, Function | MediumTest | Level3)
{
    sem_t sem;
    int semValue = 0;

    // = SEM_VALUE_MAX
    printf("> SEM_VALUE_MAX = %d\n", SEM_VALUE_MAX);
    TEST_ASSERT_NOT_EQUAL(-1, sem_init(&sem, 0, SEM_VALUE_MAX));
    if (sem_post(&sem) == -1) {     // not support OVERFLOW
        TEST_ASSERT_EQUAL_INT(errno, EOVERFLOW);
    } else {
        LOG("> sem_post return unexpected");
    }

    TEST_ASSERT_EQUAL_INT(0, sem_destroy(&sem));
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_TIMEDWAIT_0200
 * @tc.name     sem_timedwait get semaphore, wait time abnormal, tv_nsec less than 0
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemTimedwaitAbnormalA, Function | MediumTest | Level3)
{
    struct timespec ts = {0};
    sem_t sem;
    int semValue = 0;

    TEST_ASSERT_EQUAL_INT(0, sem_init(&sem, 0, 0));

    ts.tv_sec = time(NULL);
    ts.tv_nsec = -2;
    if (sem_timedwait(&sem, &ts) == -1) {
        TEST_ASSERT_EQUAL_INT(errno, EINVAL);
    } else {
        LOG("> sem_timedwait return unexpected");
    }

    TEST_ASSERT_EQUAL_INT(0, sem_getvalue(&sem, &semValue));
    TEST_ASSERT_EQUAL_INT(0, semValue);
    TEST_ASSERT_EQUAL_INT(0, sem_destroy(&sem));
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_TIMEDWAIT_0300
 * @tc.name     sem_timedwait get semaphore, wait time abnormal
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemTimedwaitAbnormalB, Function | MediumTest | Level3)
{
    struct timespec ts = {0};
    sem_t sem;
    int semValue = 0;

    TEST_ASSERT_EQUAL_INT(0, sem_init(&sem, 0, 0));

    ts.tv_sec = time(NULL);
    ts.tv_nsec = KERNEL_NS_PER_SECOND;
    if (sem_timedwait(&sem, &ts) == -1) {
        TEST_ASSERT_EQUAL_INT(errno, EINVAL);
    } else {
        LOG("> sem_timedwait return unexpected");
    }

    TEST_ASSERT_EQUAL_INT(0, sem_getvalue(&sem, &semValue));
    TEST_ASSERT_EQUAL_INT(0, semValue);
    TEST_ASSERT_EQUAL_INT(0, sem_destroy(&sem));
}

RUN_TEST_SUITE(SemAbnormalTestSuite);

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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/resource.h>
#include <fcntl.h>
#include "ohos_types.h"
#include "hctest.h"
#include "utils.h"
#include "KernelConstants.h"
#include "log.h"
#include <sched.h>

LITE_TEST_SUIT(SCHED, ProcessSchedApi, ProcessSchedApiTestSuite);

static BOOL ProcessSchedApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL ProcessSchedApiTestSuiteTearDown(void)
{
    LOG("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_GET_PRIORITY_MAX_0200
 * @tc.name     sched_get_priority_max api error test with unsupport policy.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ProcessSchedApiTestSuite, testSchedGetPriorityMaxError0200, Function | MediumTest | Level3)
{
    // SCHED_RR is 2
    int invalidPolicy[] = {SCHED_FIFO, SCHED_OTHER, SCHED_BATCH, SCHED_IDLE, SCHED_DEADLINE, SCHED_RESET_ON_FORK};
    int testLoop = sizeof(invalidPolicy)/sizeof(int);
    for (int i = 0; i < testLoop; i++) {
        errno = 0;
        int prio = sched_get_priority_max(invalidPolicy[i]);
        EXPECT_EQ(prio, -1);
        EXPECT_EQ(errno, EINVAL);
    }
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_GET_PRIORITY_MAX_0300
 * @tc.name     sched_get_priority_max api error test with invalid policy.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ProcessSchedApiTestSuite, testSchedGetPriorityMaxError0300, Function | MediumTest | Level3)
{
    // SCHED_RR is 2
    int invalidPolicyVal;
    int prio;

    invalidPolicyVal = -GetRandom(10000);
    errno = 0;
    prio = sched_get_priority_max(invalidPolicyVal);
    EXPECT_EQ(prio, -1);
    EXPECT_EQ(errno, EINVAL);

    invalidPolicyVal = GetRandom(10000) + SCHED_DEADLINE;
    errno = 0;
    prio = sched_get_priority_max(invalidPolicyVal);
    EXPECT_EQ(prio, -1);
    EXPECT_EQ(errno, EINVAL);
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_GET_PRIORITY_MIN_0200
 * @tc.name     sched_get_priority_min api error test with unsupport policy.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ProcessSchedApiTestSuite, testSchedGetPriorityMinError0200, Function | MediumTest | Level3)
{
    //  SCHED_RR is 2
    int invalidPolicy[] = {SCHED_FIFO, SCHED_OTHER, SCHED_BATCH, SCHED_IDLE, SCHED_DEADLINE, SCHED_RESET_ON_FORK};
    int testLoop = sizeof(invalidPolicy)/sizeof(int);
    for (int i = 0; i < testLoop; i++) {
        errno = 0;
        int prio = sched_get_priority_min(invalidPolicy[i]);
        EXPECT_EQ(prio, -1);
        EXPECT_EQ(errno, EINVAL);
    }
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_GET_PRIORITY_MIN_0300
 * @tc.name     sched_get_priority_min api error test with invalid policy.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ProcessSchedApiTestSuite, testSchedGetPriorityMinError0300, Function | MediumTest | Level3)
{
    int invalidPolicyVal;
    int prio;
    invalidPolicyVal = -GetRandom(10000);
    errno = 0;
    prio = sched_get_priority_min(invalidPolicyVal);
    EXPECT_EQ(prio, -1);
    EXPECT_EQ(errno, EINVAL);

    invalidPolicyVal = GetRandom(10000) + SCHED_DEADLINE;
    errno = 0;
    prio = sched_get_priority_min(invalidPolicyVal);
    EXPECT_EQ(prio, -1);
    EXPECT_EQ(errno, EINVAL);
}

RUN_TEST_SUITE(ProcessSchedApiTestSuite);

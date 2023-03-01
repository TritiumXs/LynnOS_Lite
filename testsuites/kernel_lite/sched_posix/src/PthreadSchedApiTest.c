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
#include <pthread.h>
#include <semaphore.h>
#include <sys/resource.h>
#include <fcntl.h>
#include "ohos_types.h"
#include "hctest.h"
#include "utils.h"
#include "KernelConstants.h"
#include "log.h"

#define GetParam()  30    // todo sched_priority

LITE_TEST_SUIT(SCHED, PthreadSchedApi, PthreadSchedApiTestSuite);

static BOOL PthreadSchedApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL PthreadSchedApiTestSuiteTearDown(void)
{
    LOG("+-------------------------------------------+\n");
    return TRUE;
}

// global variables used to communicate between threads
static int g_policy = 0;
static int g_prioriy = 0;

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_INHERIT_0100
 * @tc.name     test the default value of inheritsched.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrGetInheritsched, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    ASSERT_EQ(pthread_attr_init(&attr), 0);

    int inheritsched = -1;
    int rt = pthread_attr_getinheritsched(&attr, &inheritsched);
    EXPECT_EQ(rt, 0);
    EXPECT_EQ(inheritsched, PTHREAD_INHERIT_SCHED);
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_INHERIT_0200
 * @tc.name     test set and get inheritsched.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrSetInheritsched, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    ASSERT_EQ(pthread_attr_init(&attr), 0);

    int rt = pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
    EXPECT_EQ(rt, 0);
    int inheritsched = -1;
    rt = pthread_attr_getinheritsched(&attr, &inheritsched);
    EXPECT_EQ(rt, 0);
    EXPECT_EQ(inheritsched, PTHREAD_INHERIT_SCHED);

    rt = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    EXPECT_EQ(rt, 0);
    inheritsched = -1;
    rt = pthread_attr_getinheritsched(&attr, &inheritsched);
    EXPECT_EQ(rt, 0);
    EXPECT_EQ(inheritsched, PTHREAD_EXPLICIT_SCHED);
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_INHERIT_0300
 * @tc.name     pthread_attr_setinheritsched error test.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrSetInheritschedError, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    ASSERT_EQ(pthread_attr_init(&attr), 0);

    int n = -GetRandom(100);
    int rt = pthread_attr_setinheritsched(&attr, n);
    EXPECT_EQ(rt, EINVAL);
    n = 2 + GetRandom(100);
    rt = pthread_attr_setinheritsched(&attr, n);
    EXPECT_EQ(rt, EINVAL);
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_SCHEDPARAM_0200
 * @tc.name     test set and get sched param.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrSetSchedParam, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    ASSERT_EQ(pthread_attr_init(&attr), 0);

    struct sched_param param = {0};
    int rt = pthread_attr_getschedparam(&attr, &param);
    EXPECT_EQ(rt, 0);

    param.sched_priority = 22;
    rt = pthread_attr_setschedparam(&attr, &param);
    EXPECT_EQ(rt, 0);

    rt = pthread_attr_getschedparam(&attr, &param);
    EXPECT_EQ(rt, 0);
    EXPECT_EQ(param.sched_priority, 22);
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_SCHEDPOLICY_0100
 * @tc.name     test the default value of sched policy.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrGetSchedPolicy, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    ASSERT_EQ(pthread_attr_init(&attr), 0);

    int policy = -1;
    int rt = pthread_attr_getschedpolicy(&attr, &policy);
    EXPECT_EQ(rt, 0);
    EXPECT_EQ(policy, SCHED_RR);
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_SCHEDPOLICY_0300
 * @tc.name     pthread_attr_setschedpolicy error test.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrSetSchedPolicyError, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    ASSERT_EQ(pthread_attr_init(&attr), 0);
    int rt;

    // SCHED_FIFO is 1, and SCHED_RR is 2
    /*const int testLoop = 7;*/
    int invalidPolicy[7] = {SCHED_OTHER, SCHED_BATCH, SCHED_IDLE, SCHED_DEADLINE, SCHED_RESET_ON_FORK};
    invalidPolicy[5] = -GetRandom(10000);
    invalidPolicy[6] = GetRandom(10000) + 6;
    for (int i = 0; i < 7; i++) {
        rt = pthread_attr_setschedpolicy(&attr, invalidPolicy[i]);
        EXPECT_EQ(rt, EINVAL);
    }
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_SCOPE_0100
 * @tc.name     test the default value of sched scope.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrGetScope, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    ASSERT_EQ(pthread_attr_init(&attr), 0);

    int scope = -1;
    int rt = pthread_attr_getscope(&attr, &scope);
    EXPECT_EQ(rt, 0);
    EXPECT_EQ(scope, PTHREAD_SCOPE_SYSTEM);
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_SCOPE_0200
 * @tc.name     test set and get scope.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrSetScope, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    ASSERT_EQ(pthread_attr_init(&attr), 0);

    int rt = pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
    EXPECT_EQ(rt, ENOTSUP); // liteos support PTHREAD_SCOPE_SYSTEM only

    rt = pthread_attr_setscope(&attr, -GetRandom(10000));
    EXPECT_EQ(rt, EINVAL);

    rt = pthread_attr_setscope(&attr, GetRandom(10000) + 2);
    EXPECT_EQ(rt, EINVAL);

    int scope = -1;
    rt = pthread_attr_getscope(&attr, &scope);
    EXPECT_EQ(rt, 0);
    EXPECT_EQ(scope, PTHREAD_SCOPE_SYSTEM);
}

RUN_TEST_SUITE(PthreadSchedApiTestSuite);

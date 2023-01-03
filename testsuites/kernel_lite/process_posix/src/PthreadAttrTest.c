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
#include "log.h"

LITE_TEST_SUIT(PROCESS, PthreadAttr, PthreadAttrTestSuite);

static BOOL PthreadAttrTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL PthreadAttrTestSuiteTearDown(void)
{
    LOG("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_ATTR_SETDETACHSTATE_0100
 * @tc.name     basic test about pthread_attr_setdetachstate
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadAttrTestSuite, testPthreadAttrSetdetachstate, Function | MediumTest | Level3)
{
    pthread_t tid;
    pthread_attr_t attr;
    int param;

    // PTHREAD_CREATE_DETACHED
    EXPECT_EQ(pthread_attr_init(&attr), 0);
    EXPECT_EQ(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED), 0);
    EXPECT_EQ(pthread_attr_getdetachstate(&attr, &param), 0);
    EXPECT_EQ(param, PTHREAD_CREATE_DETACHED);

    ASSERT_EQ(pthread_create(&tid, &attr, ThreadPublic, NULL), 0);
    EXPECT_EQ(pthread_attr_destroy(&attr), 0);
    EXPECT_EQ(pthread_join(tid, NULL), EINVAL);

    // PTHREAD_CREATE_JOINABLE
    EXPECT_EQ(pthread_attr_init(&attr), 0);
    EXPECT_EQ(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE), 0);
    EXPECT_EQ(pthread_attr_getdetachstate(&attr, &param), 0);
    EXPECT_EQ(param, PTHREAD_CREATE_JOINABLE);

    ASSERT_EQ(pthread_create(&tid, &attr, ThreadPublic, NULL), 0);
    EXPECT_EQ(pthread_attr_destroy(&attr), 0);
    EXPECT_EQ(pthread_join(tid, NULL), 0);
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_ATTR_SETSTACKSIZE_0200
 * @tc.name     test pthread_attr_setstacksize EINVAL
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadAttrTestSuite, testPthreadAttrSetstacksizeEINVAL, Function | MediumTest | Level3)
{
    pthread_attr_t attr;
    size_t stackSize;

    EXPECT_EQ(pthread_attr_init(&attr), 0);
    EXPECT_EQ(pthread_attr_getstacksize(&attr, &stackSize), 0);
    stackSize = PTHREAD_STACK_MIN - 1;
    LOG("PTHREAD_STACK_MIN = %d", PTHREAD_STACK_MIN);
    EXPECT_EQ(pthread_attr_setstacksize(&attr, stackSize), EINVAL);
    EXPECT_EQ(pthread_attr_destroy(&attr), 0);
}

RUN_TEST_SUITE(PthreadAttrTestSuite);


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

#include "AlarmTest.h"

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include "log.h"
#include "utils.h"

#define true 1

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(TIME, TimeAlarmTest, TimeAlarmTestSuite);

static BOOL TimeAlarmTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL TimeAlarmTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number  SUB_KERNEL_TIME_API_TIMER_CREATE_0500
 * @tc.name    timer_create function errno for EINVAL test
 * @tc.desc    [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(TimeAlarmTestSuite, testTimerCreateEINVAL, Function | MediumTest | Level4)
{
    timer_t tid = NULL;
    clockid_t clockid = GetRandom(2048);

    EXPECT_EQ(timer_create(clockid, NULL, &tid), -1);
    EXPECT_EQ(errno, EINVAL);
}

RUN_TEST_SUITE(TimeAlarmTestSuite);

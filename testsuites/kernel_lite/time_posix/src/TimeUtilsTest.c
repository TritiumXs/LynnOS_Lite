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
#include <stdlib.h>
#include <time.h>

#include <limits.h>
#include "log.h"

#include "utils.h"

#include "ohos_types.h"
#include "hctest.h"

time_t g_time = 18880;
size_t g_zero = 0;

LITE_TEST_SUIT(TIME, TimeUtilsTest, TimeUtilsTestSuite);

static BOOL TimeUtilsTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL TimeUtilsTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
* @tc.number     SUB_KERNEL_TIME_API_MKTIME_0100
* @tc.name       test mktime api
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(TimeUtilsTestSuite, testMktime, Function | MediumTest | Level2)
{
    struct tm *localTime;
    struct tm timeptr = {0};
    timeptr.tm_sec = 0;
    timeptr.tm_min = 10;
    timeptr.tm_hour = 10;
    timeptr.tm_mday = 9;
    timeptr.tm_mon = 7;
    timeptr.tm_year = 120;
    timeptr.tm_wday = 7;
    EXPECT_EQ(mktime(&timeptr), 1596967800);

    localTime = localtime(&g_time);
    ASSERT_NE(NULL, localTime);
    time_t timep = mktime(localTime);
    EXPECT_EQ(timep, 18880);
}

RUN_TEST_SUITE(TimeUtilsTestSuite);

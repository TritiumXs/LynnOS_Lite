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
#define SECS_PER_MIN 60
#define INIT_TM(tmSt, year, mon, day, hour, min, sec, wday) \
	    do {                                                    \
		            (tmSt).tm_sec = (sec);                             \
		            (tmSt).tm_min = (min);                             \
		            (tmSt).tm_hour = (hour);                           \
		            (tmSt).tm_mday = (day);                            \
		            (tmSt).tm_mon = (mon);                             \
		            (tmSt).tm_year = (year) - 1900;                      \
		            (tmSt).tm_wday = wday;                                \
		            (tmSt).__tm_gmtoff = 0;                               \
		            (tmSt).__tm_zone = "";                              \
		        } while (0)
/**
* @tc.number     SUB_KERNEL_TIME_API_MKTIME_0100
* @tc.name       test mktime api
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(TimeUtilsTestSuite, testMktime, Function | MediumTest | Level2)
{
    struct tm *localTime;
    struct tm timeptr = {0};
    struct timeval tv;
    struct timezone tz;
	
    int ret = gettimeofday(&tv, &tz);
    EXPECT_EQ(0, ret);
    long sysTimezone = (long)(-tz.tz_minuteswest) * SECS_PER_MIN;
    LOG("\n system timezone = %ld\n", sysTimezone);
    INIT_TM(timeptr, 2000, 6, 9, 10, 10, 0, 7);
    time_t timeRet = mktime(&timeptr);
    EXPECT_EQ(sysTimezone, timeptr.__tm_gmtoff);
    EXPECT_EQ(963137400 - timeptr.__tm_gmtoff, timeRet);
    localTime = localtime(&g_time);
    ASSERT_NE(NULL, localTime);
    time_t timep = mktime(localTime);
    EXPECT_EQ(timep, 18880);
}

RUN_TEST_SUITE(TimeUtilsTestSuite);

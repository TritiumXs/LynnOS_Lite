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

#include <time.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "log.h"
#include "utils.h"
#include "ClockID.h"

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(TIME, TimeClockTimeTest, TimeClockTimeTestSuite);

static BOOL TimeClockTimeTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL TimeClockTimeTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_TIME_API_CLOCK_GETTIME_0100
 * @tc.name   test all supported clockid of clock_gettime
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(TimeClockTimeTestSuite, testClockGettimeAll, Function | MediumTest | Level1)
{
    clockid_t cid = CLOCK_REALTIME;
    const char* cname = ALL_CLOCKS_NAME[cid];

    struct timespec time1 = {0, 0};
    int rt = clock_gettime(cid, &time1);
    if (rt == 0) {
        LOG("clock_gettime(%s) : tv_sec=%ld, tv_nsec=%ld\n", cname, time1.tv_sec, time1.tv_nsec);
    } else {
        LOG("%s return error, rt=%d, errno=%d:%s\n", cname, rt, errno, strerror(errno));
    }
    EXPECT_EQ(rt, 0);
}

/**
 * @tc.number SUB_KERNEL_TIME_API_CLOCK_SETTIME_0100
 * @tc.name   test clock_settime basic
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(TimeClockTimeTestSuite, testClockSettime, Function | MediumTest | Level1)
{
    struct timespec time1 = {0, 0};
    int rt = clock_gettime(CLOCK_REALTIME, &time1);
    ASSERT_EQ(rt, 0);
    LOG("current time: sec=%llu, nsec=%ld", time1.tv_sec, time1.tv_nsec);
    time_t sec = time1.tv_sec;
    time1.tv_sec -= 1;
    time1.tv_nsec = 1;
    rt = clock_settime(CLOCK_REALTIME, &time1);
    ASSERT_EQ(rt, 0);
    sleep(1);
    rt = clock_gettime(CLOCK_REALTIME, &time1);
    ASSERT_EQ(rt, 0);
    ASSERT_EQ(sec, time1.tv_sec);
}

/**
* @tc.number     SUB_KERNEL_TIME_API_GETTIMEOFDAY_0100
* @tc.name       test gettimeofday api
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(TimeClockTimeTestSuite, testGettimeofday, Function | MediumTest | Level1) {
    int sleepSec = 1;
    struct timeval tvalStart = {0};
    struct timeval tvalEnd = {0};
    struct timezone tzone;

    int ret1 = gettimeofday(&tvalStart, &tzone);
    sleep(sleepSec);
    int ret2 = gettimeofday(&tvalEnd, &tzone);
    EXPECT_EQ(0, ret1);
    EXPECT_EQ(0, ret2);
    EXPECT_TRUE((tvalEnd.tv_sec - tvalStart.tv_sec) >= sleepSec);
    EXPECT_TRUE((tvalEnd.tv_sec - tvalStart.tv_sec) < sleepSec+1);
}

/**
* @tc.number     SUB_KERNEL_TIME_API_SETTIMEOFDAY_0100
* @tc.name       test settimeofday api
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(TimeClockTimeTestSuite, testSettimeofday, Function | MediumTest | Level1) {
    int setSec = 100;
    int sleepSec = 2;
    struct timeval tvalStart = {0};
    struct timeval tvalEnd = {0};
    struct timeval set = {.tv_sec = setSec, .tv_usec = 0};

    int ret1 = settimeofday(&set, NULL);
    int ret2 = gettimeofday(&tvalStart, NULL);
    sleep(sleepSec);
    int ret3 = gettimeofday(&tvalEnd, NULL);
    EXPECT_EQ(0, ret1);
    EXPECT_EQ(0, ret2);
    EXPECT_EQ(0, ret3);
    EXPECT_EQ(setSec, tvalStart.tv_sec);
    EXPECT_TRUE((tvalEnd.tv_sec - tvalStart.tv_sec) >= sleepSec);
    EXPECT_TRUE((tvalEnd.tv_sec - tvalStart.tv_sec) < sleepSec+1);
}

RUN_TEST_SUITE(TimeClockTimeTestSuite);

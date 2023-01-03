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
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include "log.h"
#include "utils.h"
#include "ClockID.h"

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(TIME, TimeSleepTest, TimeSleepTestSuite);

static BOOL TimeSleepTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL TimeSleepTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_TIME_API_CLOCK_NANOSLEEP_0200
 * @tc.name   clock_nanosleep fail test - non-support clock_id
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(TimeSleepTestSuite, testClockNanosleepInvalidID, Reliability | SmallTest | Level2)
{
    clockid_t cid = 1;
    const char* cname = ALL_CLOCKS_NAME[cid];
    printf("test %s", cname);
    struct timespec req = {0, 100};
    struct timespec rem = {0};
    int rt = clock_nanosleep(cid, 0, &req, &rem);
    if (cid == CLOCK_SGI_CYCLE) {
        ASSERT_EQ(rt, EINVAL);
    } 
}

/**
 * @tc.number SUB_KERNEL_TIME_API_CLOCK_NANOSLEEP_0300
 * @tc.name   clock_nanosleep fail test - invalid parameter
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(TimeSleepTestSuite, testClockNanosleepInvalidPara, Reliability | SmallTest | Level2)
{
    struct timespec req = {0, 100};
    struct timespec rem = {0};
    int rt;

    // invlid clock_id
    int id = GetRandom(1000) + 12;
    LOG("check invlid clockid: %d...", id);
    rt = clock_nanosleep(id, 0, &req, &rem);
    EXPECT_EQ(rt, EINVAL);

    id = -GetRandom(1000) - 12;
    LOG("check invlid clockid: %d...", id);
    rt = clock_nanosleep(id, 0, &req, &rem);
    EXPECT_EQ(rt, EINVAL);

    // invlid flag
    int flag = TIMER_ABSTIME;
    LOG("check invlid flag: %d...", flag);
    rt = clock_nanosleep(CLOCK_REALTIME, flag, &req, &rem);
    EXPECT_EQ(rt, ENOTSUP);
    flag = GetRandom(100) + 1;
    LOG("check invlid flag: %d...", flag);
    rt = clock_nanosleep(CLOCK_REALTIME, flag, &req, &rem);
    EXPECT_EQ(rt, EINVAL);
    flag = -GetRandom(100) - 1;
    LOG("check invlid flag: %d...", flag);
    rt = clock_nanosleep(CLOCK_REALTIME, flag, &req, &rem);
    EXPECT_EQ(rt, EINVAL);

    // invlid timespec
    req.tv_sec = -1;
    req.tv_nsec = 1;
    LOG("check invlid timespec: tv_sec=-1 ...");
    rt = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);
    EXPECT_EQ(errno, EINVAL);
    req.tv_sec = 1;
    req.tv_nsec = -1;
    LOG("\ncheck invlid timespec: tv_nsec=-1 ...");
    rt = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);
    EXPECT_EQ(errno, EINVAL);
    req.tv_sec = 1;
    req.tv_nsec = 1000 * 1000 * 1000 + 1;
    LOG("check invlid timespec: tv_nsec overflow ...");
    rt = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);
    EXPECT_EQ(errno, EINVAL);

    // invlid remain
    // para not used, so not tested.
}

RUN_TEST_SUITE(TimeSleepTestSuite);

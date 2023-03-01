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

#include <sys/uio.h>

#include <grp.h>
#include <shadow.h>
#include <pwd.h>
#include <inttypes.h>
#include <iconv.h>

#include "IoTest.h"
#include <securec.h>

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(IO, IoOther, IoOtherTestSuite);

static BOOL IoOtherTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IoOtherTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_IO_OTHER_0900
 * @tc.name   strptime basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoOtherTestSuite, testStrptime, Function | MediumTest | Level1)
{
    struct tm tm;
    memset_s(&tm, sizeof(struct tm), 0, sizeof(struct tm));
    char *ret = strptime("2020-10-29 21:24:00abc", "%Y-%m-%d %H:%M:%S", &tm);
    EXPECT_STREQ(ret, "abc");
    EXPECT_EQ(tm.tm_year, 120);
    EXPECT_EQ(tm.tm_mon, 9);
    EXPECT_EQ(tm.tm_mday, 29);
    EXPECT_EQ(tm.tm_hour, 21);
    EXPECT_EQ(tm.tm_min, 24);

    ret = strptime("14 Oct October 20 09:24:00 Sat Saturday 363", "%d %b %B %y %I:%M:%S %a %A %j", &tm);
    EXPECT_STREQ(ret, "");
    EXPECT_EQ(tm.tm_year, 120);
    EXPECT_EQ(tm.tm_mon, 9);
    EXPECT_EQ(tm.tm_mday, 14);
    EXPECT_EQ(tm.tm_hour, 9);
    EXPECT_EQ(tm.tm_wday, 6);
    EXPECT_EQ(tm.tm_yday, 362);
}

RUN_TEST_SUITE(IoOtherTestSuite);

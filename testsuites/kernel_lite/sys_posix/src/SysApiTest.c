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
#include <locale.h>
#include <search.h>
#include <err.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include "log.h"
#include "utils.h"
#include "KernelConstants.h"

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(SYSTEM, SysApiTest, SysApiTestSuite);

static BOOL SysApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL SysApiTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
* @tc.number    SUB_KERNEL_SYS_STRERROR_0100
* @tc.name      test strerror
* @tc.desc      [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(SysApiTestSuite, testStrerror, Function | MediumTest | Level1)
{
    for (int i = 0; i < 15; i++) {
        LOG("%d : %s", i, strerror(i));
    }
    EXPECT_STREQ("No error information", strerror(-1));
    EXPECT_STREQ("No error information", strerror(0));
    EXPECT_STREQ("No such file or directory", strerror(2));
    EXPECT_STREQ("No child process", strerror(10));
    EXPECT_STREQ("No error information", strerror(65536));
}

RUN_TEST_SUITE(SysApiTestSuite);

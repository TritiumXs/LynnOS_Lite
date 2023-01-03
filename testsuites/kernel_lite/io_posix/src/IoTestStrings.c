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

#include <strings.h>

#include "IoTest.h"

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(IO, IoStringsApi, IoStringsApiTestSuite);

static BOOL IoStringsApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IoStringsApiTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_IO_STRINGS_0200
 * @tc.name   strncasecmp basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringsApiTestSuite, testStrncasecmp, Function | MediumTest | Level1)
{
    int ret = strncasecmp("abcdefg", "abcdEFg", 7);
    TEST_ASSERT_EQUAL_INT(0, ret);

    ret = strncasecmp("abcdefg", "abcdEF", 7);
    TEST_ASSERT_GREATER_THAN_INT(0, ret);

    ret = strncasecmp("abcdef", "abcdEFg", 7);
    TEST_ASSERT_LESS_THAN_INT(0, ret);
}

/**
 * @tc.number SUB_KERNEL_IO_STRINGS_0300
 * @tc.name   strcasecmp basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringsApiTestSuite, testStrcasecmp, Function | MediumTest | Level1)
{
    int ret = strcasecmp("abcdefg", "abcdEFg");
    TEST_ASSERT_EQUAL_INT(0, ret);

    ret = strcasecmp("abcdefg", "abcdEF");
    TEST_ASSERT_GREATER_THAN_INT(0, ret);

    ret = strcasecmp("abcdef", "abcdEFg");
    TEST_ASSERT_LESS_THAN_INT(0, ret);
}

RUN_TEST_SUITE(IoStringsApiTestSuite);

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
#include <unistd.h>
#include <math.h>
#include "log.h"
#include "inttypes.h"

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(MATH, MathStdApi, MathStdApiTestSuite);

static BOOL MathStdApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL MathStdApiTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
* @tc.number     SUB_KERNEL_MATH_STD_STDLIB_0100
* @tc.name       test abs api
* @tc.desc       [C- SOFTWARE -0100]
*/
LITE_TEST_CASE(MathStdApiTestSuite, testAbs, Function | MediumTest | Level1)
{
    const int testCount = 3;
    int testValues[] = {-3, 0, 3};
    int expected[] = {3, 0, 3};
    int ret;
    for (int i = 0; i < testCount; ++i) {
        ret = abs(testValues[i]);
        TEST_ASSERT_EQUAL_INT(ret, expected[i]);
    }
}

/**
* @tc.number SUB_KERNEL_MATH_STD_LLABS_0100
* @tc.name test llabs api
* @tc.desc [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathStdApiTestSuite, testllabs, Function | MediumTest | Level1)
{
    const int testCount = 3;
    intmax_t testValues[] = {2147483649, -2147483649, 0};
    intmax_t expected[] = {2147483649, 2147483649, 0};

    long long ret;
    for (int i = 0; i < testCount; ++i) {
        ret = llabs(testValues[i]);
        TEST_ASSERT_EQUAL_INT(ret, expected[i]);
    }
}

RUN_TEST_SUITE(MathStdApiTestSuite);

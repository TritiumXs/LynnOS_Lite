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
#include <limits.h>
#include "log.h"
#include "utils.h"

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(MATH, MathApi, MathApiTestSuite);

static BOOL MathApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL MathApiTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
* @tc.number     SUB_KERNEL_MATH_MATH_SIGNBIT_0100
* @tc.name       test signbit api
* @tc.desc       [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathApiTestSuite, testsignbit, Function | MediumTest | Level1)
{
    const int testCount = 3;
    float testValues[] = {3.000001, -3.000001, 0.0};
    float expected[] = {0, 1, 0};
    float ret;
    for (int i = 0; i < testCount; ++i) {
        ret = signbit(testValues[i]);
        TEST_ASSERT_EQUAL_INT(ret, expected[i]);
    }
}

RUN_TEST_SUITE(MathApiTestSuite);

void MathApiTest()
{
    RUN_ONE_TESTCASE(testsignbit);
}

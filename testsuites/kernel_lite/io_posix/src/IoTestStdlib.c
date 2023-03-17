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

#include <stdlib.h>

#include "log.h"
#include "IoTest.h"

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(IO, IoStdlibApi, IoStdlibApiTestSuite);

static BOOL IoStdlibApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IoStdlibApiTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_0400
 * @tc.name   strtof basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtof, Function | MediumTest | Level1)
{
    char *endPtr = NULL;
    float ret = strtof("  -3.40E+38 hello", &endPtr);
    EXPECT_FLOAT_EQ(ret, -3.40E+38);
    EXPECT_STREQ(endPtr, " hello");

    ret = strtof("  3.40E+38 ===", &endPtr);
    EXPECT_FLOAT_EQ(ret, 3.40E+38);
    EXPECT_STREQ(endPtr, " ===");

    ret = strtof("-9.6e17  this   123", &endPtr);
    EXPECT_FLOAT_EQ(ret, -9.6e17);
    EXPECT_STREQ(endPtr, "  this   123");

    ret = strtof("this is string", &endPtr);
    EXPECT_FLOAT_EQ(ret, 0);
    EXPECT_STREQ(endPtr, "this is string");
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_0500
 * @tc.name   strtod basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtod, Function | MediumTest | Level1)
{
    char *endPtr = NULL;
    double ret = strtod("  -1.79E+308 hello", &endPtr);
    EXPECT_DOUBLE_EQ(ret, -1.79E+308);
    EXPECT_STREQ(endPtr, " hello");

    ret = strtod("1.79E+308  ===", &endPtr);
    EXPECT_DOUBLE_EQ(ret, 1.79E+308);
    EXPECT_STREQ(endPtr, "  ===");

    ret = strtod("-9.6e17  this   123", &endPtr);
    EXPECT_DOUBLE_EQ(ret, -9.6e17);
    EXPECT_STREQ(endPtr, "  this   123");

    ret = strtod("this is string", &endPtr);
    EXPECT_DOUBLE_EQ(ret, 0);
    EXPECT_STREQ(endPtr, "this is string");
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_0600
 * @tc.name   strtold basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtold, Function | MediumTest | Level1)
{
    char *endPtr = NULL;
    long double ret = strtold("  2.22507e-308 hello", &endPtr);
    EXPECT_STREQ(endPtr, " hello");

    ret = strtold("  1.79769e+308 ===", &endPtr);
    EXPECT_STREQ(endPtr, " ===");

    ret = strtold("-9.6e17  this   123", &endPtr);
    EXPECT_DOUBLE_EQ(ret, -9.6e17);
    EXPECT_STREQ(endPtr, "  this   123");

    ret = strtold("this is string", &endPtr);
    EXPECT_DOUBLE_EQ(ret, 0);
    EXPECT_STREQ(endPtr, "this is string");
}


/**
 * @tc.number SUB_KERNEL_IO_STDLIB_0700
 * @tc.name   strtol basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtol, Function | MediumTest | Level1)
{
    char nPtr[] = "12 0110 0XDEFE 0666 -1.6";
    char *endPtr = NULL;
    long ret = strtol(nPtr, &endPtr, 10); // decimal
    EXPECT_EQ(ret, 12);
    EXPECT_STREQ(endPtr, " 0110 0XDEFE 0666 -1.6");

    ret = strtol(endPtr, &endPtr, 2); // binary
    EXPECT_EQ(ret, 6);
    EXPECT_STREQ(endPtr, " 0XDEFE 0666 -1.6");

    ret = strtol(endPtr, &endPtr, 16); // hexadecimal
    EXPECT_EQ(ret, 0XDEFE);
    EXPECT_STREQ(endPtr, " 0666 -1.6");


    ret = strtol(endPtr, &endPtr, 8); // octal
    EXPECT_EQ(ret, 0666);
    EXPECT_STREQ(endPtr, " -1.6");

    ret = strtol(endPtr, &endPtr, 65); // Invalid number format
    EXPECT_EQ(ret, 0);
    EXPECT_STREQ(endPtr, " -1.6");

    ret = strtol(endPtr, &endPtr, 0); // decimal
    EXPECT_EQ(ret, -1);
    EXPECT_STREQ(endPtr, ".6");

    ret = strtol(endPtr, &endPtr, 10); // Invalid transform
    EXPECT_EQ(ret, 0);
    EXPECT_STREQ(endPtr, ".6");

    char nPtr1[] = "2147483647 2147483648 -2147483648 -2147483649"; // boundary value：2^31-1
    ret = strtol(nPtr1, &endPtr, 10);
    EXPECT_EQ(ret, 2147483647);
    EXPECT_STREQ(endPtr, " 2147483648 -2147483648 -2147483649");

    ret = strtol(endPtr, &endPtr, 10);
    EXPECT_EQ(ret, 2147483647);
    EXPECT_STREQ(endPtr, " -2147483648 -2147483649");

    ret = strtol(endPtr, &endPtr, 10);
    EXPECT_EQ(ret, -2147483648);
    EXPECT_STREQ(endPtr, " -2147483649");

    ret = strtol(endPtr, &endPtr, 10);
    EXPECT_EQ(ret, -2147483648);
    EXPECT_STREQ(endPtr, "");
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_0800
 * @tc.name   strtoul basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtoul, Function | MediumTest | Level1)
{
    char nPtr[] = "12 0110 0XDEFE 0666 4294967295 4294967296 12.34"; // 2^32-1
    char *endPtr = NULL;
    unsigned long ret = strtoul(nPtr, &endPtr, 10);
    EXPECT_EQ(ret, 12UL);
    EXPECT_STREQ(endPtr, " 0110 0XDEFE 0666 4294967295 4294967296 12.34");

    ret = strtoul(endPtr, &endPtr, 2);
    EXPECT_EQ(ret, 6UL);
    EXPECT_STREQ(endPtr, " 0XDEFE 0666 4294967295 4294967296 12.34");

    ret = strtoul(endPtr, &endPtr, 16);
    EXPECT_EQ(ret, 0XDEFEUL);
    EXPECT_STREQ(endPtr, " 0666 4294967295 4294967296 12.34");


    ret = strtoul(endPtr, &endPtr, 8);
    EXPECT_EQ(ret, 0666UL);
    EXPECT_STREQ(endPtr, " 4294967295 4294967296 12.34");

    ret = strtoul(endPtr, &endPtr, 0);
    EXPECT_EQ(ret, 4294967295UL);
    EXPECT_STREQ(endPtr, " 4294967296 12.34");

    ret = strtoul(endPtr, &endPtr, 0);
    EXPECT_EQ(ret, 4294967295UL);
    EXPECT_STREQ(endPtr, " 12.34");

    ret = strtoul(endPtr, &endPtr, 65);
    EXPECT_EQ(ret, 0UL);
    EXPECT_STREQ(endPtr, " 12.34");

    ret = strtoul(endPtr, &endPtr, 0);
    EXPECT_EQ(ret, 12UL);
    EXPECT_STREQ(endPtr, ".34");

    ret = strtoul(endPtr, &endPtr, 0);
    EXPECT_EQ(ret, 0UL);
    EXPECT_STREQ(endPtr, ".34");
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_1000
 * @tc.name   strtoull basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtoull, Function | MediumTest | Level1)
{
    char nPtr[] = "12 0110 0XDEFE 0666 1.6";
    char *endPtr = NULL;
    unsigned long long ret = strtoull(nPtr, &endPtr, 10);
    EXPECT_EQ(ret, 12ULL);
    EXPECT_STREQ(endPtr, " 0110 0XDEFE 0666 1.6");

    ret = strtoull(endPtr, &endPtr, 2);
    EXPECT_EQ(ret, 6ULL);
    EXPECT_STREQ(endPtr, " 0XDEFE 0666 1.6");

    ret = strtoull(endPtr, &endPtr, 16);
    EXPECT_EQ(ret, 0XDEFEULL);
    EXPECT_STREQ(endPtr, " 0666 1.6");


    ret = strtoull(endPtr, &endPtr, 8);
    EXPECT_EQ(ret, 0666ULL);
    EXPECT_STREQ(endPtr, " 1.6");

    ret = strtoull(endPtr, &endPtr, 65);
    EXPECT_EQ(ret, 0ULL);
    EXPECT_STREQ(endPtr, " 1.6");

    ret = strtoull(endPtr, &endPtr, 0);
    EXPECT_EQ(ret, 1ULL);
    EXPECT_STREQ(endPtr, ".6");

    ret = strtoull(endPtr, &endPtr, 0);
    EXPECT_EQ(ret, 0ULL);
    EXPECT_STREQ(endPtr, ".6");

    char nPtr1[] = "18446744073709551615 18446744073709551616"; // 2^64-1
    ret = strtoull(nPtr1, &endPtr, 10);
    EXPECT_EQ(ret, 18446744073709551615ULL);
    EXPECT_STREQ(endPtr, " 18446744073709551616");

    ret = strtoull(endPtr, &endPtr, 10);
    EXPECT_EQ(ret, 18446744073709551615ULL);
    EXPECT_STREQ(endPtr, "");
}

RUN_TEST_SUITE(IoStdlibApiTestSuite);

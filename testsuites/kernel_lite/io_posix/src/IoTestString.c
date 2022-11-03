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

#include "IoTest.h"
#include <string.h>

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(IO, IoStringApi, IoStringApiTestSuite);

static BOOL IoStringApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IoStringApiTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_0300
 * @tc.name   strcmp basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrcmp, Function | MediumTest | Level1)
{
    int ret = strcmp("abcdef", "ABCDEF");
    TEST_ASSERT_GREATER_THAN_INT(0, ret);

    ret = strcmp("123456", "654321");
    TEST_ASSERT_LESS_THAN_INT(0, ret);

    EXPECT_EQ(strcmp("~!@#$%^&*()_+", "~!@#$%^&*()_+"), 0);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_0400
 * @tc.name   strncmp basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrncmp, Function | MediumTest | Level1)
{
    const char dest[] = "abcdef";
    const char src[] = "ABCDEF";
    int ret = strncmp(dest, src, 1);
    TEST_ASSERT_GREATER_THAN_INT(0, ret);

    ret = strncmp("123456", "654321", 6);
    TEST_ASSERT_LESS_THAN_INT(0, ret);

    ret = strncmp("~!@#$%^&*()_+", "~!@#$%^&*()_+", 8);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_0700
 * @tc.name   stpcpy basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStpcpy, Function | MediumTest | Level1)
{
    const char *src = "hello world !";
    char dest[50] = {0};
    char *ret = stpcpy(dest, src);
    ASSERT_NE(ret, NULL);
    EXPECT_STREQ(dest, src);

    const char *srcT = "~!@#$%%^&*()_+";
    char destT[50] = {0};
    ret = stpcpy(destT, srcT);
    ASSERT_NE(ret, NULL);
    EXPECT_STREQ(destT, "~!@#$%%^&*()_+");
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_0800
 * @tc.name   stpncpy basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStpncpy, Function | MediumTest | Level1)
{
    char src[] = "hello world !";
    char dest[50] = {0};
    char *ret = stpncpy(dest, src, 5);
    EXPECT_STREQ(ret, "");
    EXPECT_STREQ(dest, "hello");

    char srcT[] = "~!@#$%%^&*()_+";
    char destT[50] = {0};
    ret = stpncpy(destT, srcT, 15);
    EXPECT_STREQ(ret, "");
    EXPECT_STREQ(destT, "~!@#$%%^&*()_+");
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1000
 * @tc.name   strcpy basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrcpy, Function | MediumTest | Level1)
{
    char src[] = "hello world !";
    char dest[20] = {0};
    char *ret = strcpy(dest, src);
    EXPECT_STREQ(ret, dest);

    char srcT[] = "This is String1";
    char destT[20] = {0};
    ret = strcpy(destT, srcT);
    EXPECT_STREQ(ret, destT);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1100
 * @tc.name   strncpy basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrncpy, Function | MediumTest | Level1)
{
    char src[] = "hello world !";
    char dest[] = {0};
    char *ret = strncpy(dest, src, 6);
    EXPECT_STREQ(ret, dest);

    char destT[] = "~!@#$%^&*()_+";
    ret = strncpy(destT, "hello world", 0);
    EXPECT_STREQ(ret, destT);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1400
 * @tc.name   strcat basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrcat, Function | MediumTest | Level1)
{
    char dest[50] = {0};
    char *ret = strcat(dest, "hello world !");
    EXPECT_STREQ(ret, dest);

    char destT[50] = "hello world ! || ";
    ret = strcat(destT, "This is String1");
    EXPECT_STREQ(ret, destT);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1500
 * @tc.name   strncat basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrncat, Function | MediumTest | Level1)
{
    char dest[50] = "hello world || ";
    char *ret = strncat(dest, "This is String1", 7);
    EXPECT_STREQ(ret, "hello world || This is");

    char destT[20] = "|/*";
    ret = strncat(destT, "~!@#$%^&*()_+", 13);
    EXPECT_STREQ(ret, destT);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1600
 * @tc.name   strchr basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrchr, Function | MediumTest | Level1)
{
    char src[] = "hello !! world";
    char *ret = strchr(src, '!');
    EXPECT_STREQ(ret, "!! world");

    ret = strchr(src, '?');
    EXPECT_STREQ(ret, NULL);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1700
 * @tc.name   strrchr basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrrchr, Function | MediumTest | Level1)
{
    char src[] = "hello world & HEELO & WORLD";
    char *ret = strrchr(src, '&');
    EXPECT_STREQ(ret, "& WORLD");

    ret = strrchr(src, '?');
    EXPECT_STREQ(ret, NULL);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1800
 * @tc.name   strnlen basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrnlen, Function | MediumTest | Level1)
{
    char src[] = "hello world !";
    unsigned int ltSrcSize = strlen(src) - 2;
    unsigned int gtSrcSize = strlen(src) + 2;

    size_t ret = strnlen(src, ltSrcSize);
    EXPECT_EQ(ret, ltSrcSize);

    ret = strnlen(src, gtSrcSize);
    EXPECT_EQ(ret, strlen(src));
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1900
 * @tc.name   strtok basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrtok, Function | MediumTest | Level1)
{
    char src[] = "hello world & This is-String1";
    char *ret = strtok(src, "&");
    EXPECT_STREQ(ret, src);

    ret = strtok(NULL, "-");
    EXPECT_STREQ(ret, " This is");

    char srcS[] = "hello world !";
    ret = strtok(srcS, "?");
    EXPECT_STREQ(ret, srcS);

    char srcT[50] = {0};
    ret = strtok(srcT, "~");
    EXPECT_STREQ(ret, NULL);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_2200
 * @tc.name   strdup basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrdup, Function | MediumTest | Level1)
{
    char src[] = "hello world !";
    char *ret = strdup(src);
    EXPECT_STREQ(ret, src);

    char srcS[] = "This is String1";
    ret = strdup(srcS);
    EXPECT_STREQ(ret, "This is String1");
    free(ret);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_2500
 * @tc.name   strcspn basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrcspn, Function | MediumTest | Level1)
{
    const char dest[] = "hello world !";
    const char src[] = "!";
    size_t ret = strcspn(dest, src);
    EXPECT_EQ(ret, 12U);

    const char srcS[] = "a";
    ret = strcspn(dest, srcS);
    EXPECT_EQ(ret, 13U);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_2600
 * @tc.name   strspn basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrspn, Function | MediumTest | Level1)
{
    const char dest[] = "hello world !";
    const char src[] = "heAlo";
    size_t ret = strspn(dest, src);
    EXPECT_EQ(ret, 5U);
    EXPECT_EQ(ret, 5U);

    const char destS[] = "this is string";
    const char srcS[] = "abc";
    ret = strspn(destS, srcS);
    EXPECT_EQ(ret, 0U);

    const char srcT[] = "helo\0 wal";
    ret = strspn(dest, srcT);
    EXPECT_EQ(ret, 5U);
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_2700
 * @tc.name   strstr basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrstr, Function | MediumTest | Level1)
{
    const char dest[] = "hello world !";
    const char src[] = "heAlo";
    const char *ret = strstr(dest, src);
    EXPECT_STREQ(ret, NULL);

    const char destS[] = "string this is string";
    const char srcS[] = "string";
    ret = strstr(destS, srcS);
    EXPECT_STREQ(ret, "string this is string");

    const char srcT[] = "\0hello";
    ret = strstr(dest, srcT);
    EXPECT_STREQ(ret, "hello world !");
}

RUN_TEST_SUITE(IoStringApiTestSuite);

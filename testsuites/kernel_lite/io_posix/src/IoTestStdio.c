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
#include <stdarg.h>

#include <err.h>
#include <threads.h>

#include "libfs.h"
#include "utils.h"
#include "IoTest.h"

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(IO, IoStdio, IoStdioTestSuite);

static BOOL IoStdioTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IoStdioTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

int FormatVsnprintf(char *format, ...)
{
    va_list vArgList;
    va_start(vArgList, format);
    char str[50] = {0};
    int ret = vsnprintf(str, sizeof(str), format, vArgList);
    va_end(vArgList);
    return ret;
}

/**
 * @tc.number SUB_KERNEL_IO_STDIO_2100
 * @tc.name   vsnprintf basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdioTestSuite, testVsnprintf, Function | MediumTest | Level1)
{
    int ret = FormatVsnprintf((char *)"%s has %d words", "hello world", 11);
    TEST_ASSERT_EQUAL_INT(ret, 24);

    ret = FormatVsnprintf((char *)"%f and %c as well as %ld\n", 2.2, 'c', 6);
    TEST_ASSERT_EQUAL_INT(ret, 28);
}

RUN_TEST_SUITE(IoStdioTestSuite);

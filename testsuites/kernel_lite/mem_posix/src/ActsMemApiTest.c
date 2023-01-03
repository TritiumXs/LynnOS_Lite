/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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
#include <stdio.h>
#include <securec.h>
#include <pthread.h>
#include <time.h>
#include <sys/resource.h>
#include <limits.h>
#include "ohos_types.h"
#include "hctest.h"
#include "utils.h"
#include "log.h"

LITE_TEST_SUIT(MEM, ActsMemApi, ActsMemApiTestSuite);

static BOOL ActsMemApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL ActsMemApiTestSuiteTearDown(void)
{
    LOG("+-------------------------------------------+\n");
    return TRUE;
}

/**
* @tc.number     SUB_KERNEL_NDKAPI_MEM_MEMCHR_0100
* @tc.name       test memchr api
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(ActsMemApiTestSuite, testMemchr_0100, Function | MediumTest | Level1) {
    char srcStr[] = "this is str a;";
    char *pos = nullptr;

    LogPrint("    memchr *s:='%s' c:='a' size:='14',", srcStr);
    pos = (char *)memchr(srcStr, 'a', 14);
    LogPrint("    --> return pos:='%x'\n", pos);

    ASSERT_TRUE(pos);
}

/**
* @tc.number     SUB_KERNEL_NDKAPI_MEM_MEMCHR_1000
* @tc.name       test memchr api para len not enough
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(ActsMemApiTestSuite, testMemchr_1000, Function | MediumTest | Level1) {
    char srcStr[] = "this is str a;";
    char *pos = nullptr;

    LogPrint("    memchr *s:='%s' c:='a' size:='4',", srcStr);
    pos = (char *)memchr(srcStr, 'a', 4);
    LogPrint("    --> return pos:='%x'\n", pos);
    ASSERT_FALSE(pos);
}

/**
* @tc.number     SUB_KERNEL_NDKAPI_MEM_MEMCHR_1100
* @tc.name       test memchr api para c not found
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(ActsMemApiTestSuite, testMemchr_1100, Function | MediumTest | Level1) {
    char srcStr[] = "this is str a;";
    char *pos = nullptr;

    LogPrint("    memchr *s:='%s' c:='b' size:='14',", srcStr);
    pos = (char *)memchr(srcStr, 'b', 14);
    LogPrint("    --> return pos:='%x'\n", pos);
    ASSERT_FALSE(pos);
}

RUN_TEST_SUITE(ActsMemApiTestSuite);


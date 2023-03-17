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

LITE_TEST_SUIT(MEM, MemApi, MemApiTestSuite);

static BOOL MemApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL MemApiTestSuiteTearDown(void)
{
    LOG("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_MEM_MEMSET_0100
 * @tc.name   memset function set buffer value test
 * @tc.desc   [C-L*-311] MUST NOT alter NDK API behavior.
 */
LITE_TEST_CASE(MemApiTestSuite, testMemset, Function | MediumTest | Level1)
{
    char chr = 'A';
    int i, len, failure;
    len = GetRandom(1024);
    errno_t err = EOK;

    char buf[1024];
    err = memset_s(buf, sizeof(buf), chr, len);
    if(err != EOK) {
        LOG("memset_s failed, err = %d\n", err);
    }
    failure = 0;
    for (i = 0; i < len; i++) {
        if (buf[i] != chr) {
            failure = 1;
            break;
        }
    }
    ASSERT_TRUE(failure == 0);
}

/**
 * @tc.number SUB_KERNEL_MEM_MEMCPY_0100
 * @tc.name   memcpy function copy buffer test
 * @tc.desc   [C-L*-311] MUST NOT alter NDK API behavior.
 */
LITE_TEST_CASE(MemApiTestSuite, testMemcpy, Function | MediumTest | Level2)
{
    char chr = 'A';
    int i, len, failure;
    char src[1024];
    char dst[1024];

    len = GetRandom(1024);

    for (i = 0; i < len; i++) {
        src[i] = chr + i % 26;
    }

    memcpy(dst, src, len);
    failure = 0;
    for (i = 0; i < len; i++) {
        if (dst[i] != src[i]) {
            failure = 1;
            break;
        }
    }
    ASSERT_TRUE(failure == 0);
}

/**
 * @tc.number SUB_KERNEL_MEM_MEMMOVE_0100
 * @tc.name   memmove function move buffer test
 * @tc.desc   [C-L*-311] MUST NOT alter NDK API behavior.
 */
LITE_TEST_CASE(MemApiTestSuite, testMemmove, Function | MediumTest | Level2)
{
    char chr = 'A';
    char buf[1024];
    int i, len, failure;

    len = sizeof(buf);
    for (i = 0; i < len; i++) {
        buf[i] = chr + GetRandom(26);
    }
    memmove(&buf[0], &buf[len / 2], len / 2);

    failure = 0;
    for (i = 0; i < len / 2; i++) {
        if (buf[i] != buf[len / 2 + i]) {
            failure = 1;
            LOG("buf[i] != buf[len / 2 + i], buf[i] = %d, buf[len / 2 + i] = %d", buf[i], buf[len / 2 + i]);
            break;
        }
    }
    /* buf[i] not equal to buf[len / 2 + i] */
    ASSERT_TRUE(failure == 0);
}

/**
 * @tc.number SUB_KERNEL_MEM_MEMMOVE_0200
 * @tc.name   memmove function overlay move buffer test
 * @tc.desc   [C-L*-311] MUST NOT alter NDK API behavior.
 */
LITE_TEST_CASE(MemApiTestSuite, testMemmoveOverlay, Function | MediumTest | Level3)
{
    char chr = 'A';
    char buf[1024];
    char backup[1024];
    int i, len, failure;

    len = sizeof(buf);
    for (i = 0; i < len; i++) {
        buf[i] = chr + GetRandom(26);
        backup[i] = buf[i];
    }
    memmove(&buf[16], &buf[0], len / 2);

    failure = 0;
    for (i = 0; i < len / 2; i++) {
        if (buf[i + 16] != backup[i]) {
            failure = 1;
            LOG("buf[i + 16] != backup[i], buf[i + 16] = %d, backup[i] = %d", buf[i + 16], backup[i]);
            break;
        }
    }
    ASSERT_TRUE(failure == 0);
}


/**
 * @tc.number SUB_KERNEL_MEM_MEMCMP_0100
 * @tc.name   memmove function move buffer test
 * @tc.desc   [C-L*-311] MUST NOT alter NDK API behavior.
 */
LITE_TEST_CASE(MemApiTestSuite, testMemcmp, Function | MediumTest | Level2)
{
    char orign[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    char lt[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x77};
    char eq[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    char gt[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x99};

    int ret;
    int len = sizeof(orign);

    ret = memcmp(lt, orign, len);
    ASSERT_TRUE(ret < 0);

    ret = memcmp(eq, orign, len);
    ASSERT_TRUE(ret == 0);

    ret = memcmp(gt, orign, len);
    ASSERT_TRUE(ret > 0);

    ret = memcmp(gt, orign, 0);
    ASSERT_TRUE(ret == 0);
}

RUN_TEST_SUITE(MemApiTestSuite);

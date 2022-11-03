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
#include <setjmp.h>
#include <securec.h>
#include <pthread.h>
#include <time.h>
#include <sys/resource.h>
#include <limits.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "ohos_types.h"
#include "hctest.h"
#include "utils.h"
#include "log.h"

LITE_TEST_SUIT(PROCESS, ProcessApi, ProcessApiTestSuite);

static BOOL ProcessApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL ProcessApiTestSuiteTearDown(void)
{
    LOG("+-------------------------------------------+\n");
    return TRUE;
}

static void *ThreadFunc(void* arg)
{
    LogPrint("    This is ThreadFunc()\n");
    return NULL;
}

/**
* @tc.number     SUB_KERNEL_NDKAPI_PROCESS_PTHREAD_SETNAME_NP_1000
* @tc.name       test pthread_setname_np api para stringlong
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(ProcessApiTestSuite, testPthreadSetnameNp1000, Function | MediumTest | Level1) {
    pthread_t thisThread;
    int returnVal;

    returnVal = pthread_create(&thisThread, NULL, ThreadFunc, NULL);
    // thisThread = pthread_self();
    LogPrint("    pthread_self(),   --> return pthread_t:='%u(0x%x)'\n",
        (unsigned int)thisThread, (unsigned int)thisThread);
    
    returnVal = pthread_setname_np(thisThread, "funcThreadNamelongName");
    LogPrint("    pthread_setname_np thread:='%u(0x%x)' *name:='funcThreadNamelongName',   "
        "--> returnVal:='%d'\n", thisThread, thisThread, returnVal);
    
    EXPECT_NE(returnVal, 0);
    EXPECT_EQ(returnVal, ERANGE);
}

RUN_TEST_SUITE(ProcessApiTestSuite);


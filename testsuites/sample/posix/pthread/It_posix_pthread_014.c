/*
 * Copyright (c) 2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "It_posix_pthread.h"

static UINT32 Testcase(VOID)
{
    pthread_condattr_t condattr;
    int pshared;
    int rc;

    rc = pthread_condattr_init(&condattr);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);

    rc = pthread_condattr_getpshared(NULL, &pshared);
    ICUNIT_ASSERT_EQUAL(rc, EINVAL, rc);

    rc = pthread_condattr_getpshared(&condattr, NULL);
    ICUNIT_ASSERT_EQUAL(rc, EINVAL, rc);

    rc = pthread_condattr_getpshared(NULL, NULL);
    ICUNIT_ASSERT_EQUAL(rc, EINVAL, rc);

    rc = pthread_condattr_getpshared(&condattr, &pshared);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);
    ICUNIT_ASSERT_EQUAL(pshared, PTHREAD_PROCESS_PRIVATE, pshared);

    rc = pthread_condattr_destroy(&condattr);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);

    return LOS_OK;
}

/**
 * @tc.name: ItPosixPthread014
 * @tc.desc: Test interface pthread_condattr_getpshared
 * @tc.type: FUNC
 * @tc.require: issueI5TIRQ
 */

VOID ItPosixPthread014(VOID)
{
    TEST_ADD_CASE("ItPosixPthread014", Testcase, TEST_POSIX, TEST_PTHREAD, TEST_LEVEL2, TEST_FUNCTION);
}

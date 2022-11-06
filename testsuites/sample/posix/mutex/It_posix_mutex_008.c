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

#include "It_posix_mutex.h"

static UINT32 Testcase(VOID)
{
    pthread_mutex_t mutex;
    int rc;

    /* Initialize a mutex object */
    rc = pthread_mutex_init(&mutex, NULL);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);

    /* Acquire the mutex object using pthread_mutex_lock */
    rc = pthread_mutex_lock(&mutex);
    ICUNIT_GOTO_EQUAL(rc, 0, rc, EXIT1);

    sleep(1);

    /* Release the mutex object using pthread_mutex_unlock */
    rc = pthread_mutex_unlock(&mutex);
    ICUNIT_GOTO_EQUAL(rc, 0, rc, EXIT2);

    /* Destroy the mutex object */
    rc = pthread_mutex_destroy(&mutex);
    ICUNIT_GOTO_EQUAL(rc, 0, rc, EXIT1);

    return LOS_OK;

EXIT2:
    pthread_mutex_unlock(&mutex);

EXIT1:
    pthread_mutex_destroy(&mutex);
    return LOS_OK;
}

/**
 * @tc.name: ItPosixMux008
 * @tc.desc: Test interface pthread_mutex_lock
 * @tc.type: FUNC
 * @tc.require: issueI5WZI6
 */

VOID ItPosixMux008(void)
{
    TEST_ADD_CASE("ItPosixMux008", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}

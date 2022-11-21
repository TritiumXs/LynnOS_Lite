/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
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

#include "posix_fs_test.h"

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpenOK, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    const char tmpFileName[] = FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);
    (void)close(fd);

    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);
    (void)close(fd);
    (void)unlink(tmpFileName);

    return POSIX_FS_NO_ERROR;
}

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpenEINVAL, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    const char *tmpFileName1 = NULL;
    const char *tmpFileName2 = "/";

    fd = open(tmpFileName1, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    fd = open(tmpFileName2, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    return POSIX_FS_NO_ERROR;
}

extern struct MountPoint *g_mountPoints;
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpenENOENT, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    const char tmpFileName[] = FILE1;
    struct MountPoint *mountBak = g_mountPoints;

    g_mountPoints = NULL;
    fd = open(tmpFileName, O_CREAT | O_RDWR);
    g_mountPoints = mountBak;
    ICUNIT_ASSERT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    return POSIX_FS_NO_ERROR;
}

void posixFsOpenTest(void)
{
    RUN_ONE_TESTCASE(testFsOpenOK);
    RUN_ONE_TESTCASE(testFsOpenEINVAL);
    RUN_ONE_TESTCASE(testFsOpenENOENT);
}

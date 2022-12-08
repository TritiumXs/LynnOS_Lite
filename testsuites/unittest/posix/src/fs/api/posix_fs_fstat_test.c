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
#include "vfs_config.h"

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFstatOK, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    struct stat buf = { 0 };
    const char tmpFileName[] = FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = fstat(fd, &buf);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    (void)close(fd);
    (void)unlink(tmpFileName);

    return POSIX_FS_NO_ERROR;
}

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFstatEBADF001, Function | MediumTest | Level1)
{
    int32_t ret = 0;
    struct stat buf = { 0 };

    ret = fstat(-1, &buf);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFstatEBADF002, Function | MediumTest | Level1)
{
    int32_t ret = 0;
    struct stat buf = { 0 };

    ret = fstat(ERROR_CONFIG_NFILE_DESCRIPTORS, &buf);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFstatEBADF003, Function | MediumTest | Level1)
{
    int32_t ret = 0;
    struct stat buf = { 0 };

    ret = fstat(0, &buf);   /* 0, used for stdin */
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = 0;
    ret = fstat(1, &buf);   /* 1, used for stdin */
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = 0;
    ret = fstat(2, &buf);   /* 2, used for stdin */
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFstatEINVAL, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    struct stat *buf = NULL;
    const char tmpFileName[] = FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = fstat(fd, buf);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    (void)close(fd);
    (void)unlink(tmpFileName);

    return POSIX_FS_NO_ERROR;
}

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFstatENOENT, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    struct stat buf = { 0 };
    const char tmpFileName[] = FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    struct MountPoint *mountBak = g_mountPoints;
    g_mountPoints = NULL;
    ret = fstat(fd, &buf);
    g_mountPoints = mountBak;
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    (void)close(fd);
    (void)unlink(tmpFileName);

    return POSIX_FS_NO_ERROR;
}

void posixFsFstatTest(void)
{
    RUN_ONE_TESTCASE(testFsFstatOK);
    RUN_ONE_TESTCASE(testFsFstatEBADF001);
    RUN_ONE_TESTCASE(testFsFstatEBADF002);
    RUN_ONE_TESTCASE(testFsFstatEBADF003);
    RUN_ONE_TESTCASE(testFsFstatEINVAL);
    RUN_ONE_TESTCASE(testFsFstatENOENT);
}

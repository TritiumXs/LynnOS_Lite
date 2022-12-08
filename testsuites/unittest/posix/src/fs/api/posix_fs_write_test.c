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

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsWriteOK, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    const char tmpFileName[] = FILE1;
    const char writeBuf[TEST_BUF_SIZE] = "hello";

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = write(fd, writeBuf, TEST_BUF_SIZE);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    (void)close(fd);
    (void)unlink(tmpFileName);

    return POSIX_FS_NO_ERROR;
}

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsWriteEINVAL, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    const char tmpFileName[] = FILE1;
    const char *writeBuf1 = NULL;
    const char writeBuf2[TEST_BUF_SIZE] = "hello";

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = write(fd, writeBuf1, TEST_BUF_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = write(fd, writeBuf2, 0);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    (void)close(fd);
    (void)unlink(tmpFileName);

    return POSIX_FS_NO_ERROR;
}

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsWriteEACCES, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    const char tmpFileName[] = FILE1;
    const char writeBuf[TEST_BUF_SIZE] = "hello";

    fd = open(tmpFileName, O_CREAT | O_RDONLY);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = write(fd, writeBuf, TEST_BUF_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    (void)close(fd);
    (void)unlink(tmpFileName);

    return POSIX_FS_NO_ERROR;
}

void posixFsWriteTest(void)
{
    RUN_ONE_TESTCASE(testFsWriteOK);
    RUN_ONE_TESTCASE(testFsWriteEINVAL);
    RUN_ONE_TESTCASE(testFsWriteEACCES);
}

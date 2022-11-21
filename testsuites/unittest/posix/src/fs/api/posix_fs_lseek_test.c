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

/* *
 * @tc.number   SUB_KERNEL_FS_LSEEK_OK001
 * @tc.name     lseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsLseekOK001, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    off_t off = 0;
    const char tmpFileName[] = FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    off = lseek(fd, 0, SEEK_SET);
    ICUNIT_ASSERT_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off);

    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_LSEEK_OK002
 * @tc.name     lseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsLseekOK002, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    off_t off = 0;
    const char tmpFileName[] = FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    off = lseek(fd, 0, SEEK_CUR);
    ICUNIT_ASSERT_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off);

    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_LSEEK_OK003
 * @tc.name     lseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsLseekOK003, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    off_t off = 0;
    const char tmpFileName[] = FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    off = lseek(fd, 0, SEEK_END);
    ICUNIT_ASSERT_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off);

    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_LSEEK_EBADF001
 * @tc.name     lseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsLseekEBADF001, Function | MediumTest | Level1)
{
    off_t off = 0;

    off = lseek(-1, 0, SEEK_SET);
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL((int32_t)off, POSIX_FS_IS_ERROR, (int32_t)off);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_LSEEK_EBADF002
 * @tc.name     lseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsLseekEBADF002, Function | MediumTest | Level1)
{
    off_t off = 0;

    off = lseek(ERROR_CONFIG_NFILE_DESCRIPTORS, 0, SEEK_SET);
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL((int32_t)off, POSIX_FS_IS_ERROR, (int32_t)off);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_LSEEK_EBADF003
 * @tc.name     lseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsLseekEBADF003, Function | MediumTest | Level1)
{
    off_t off1 = 0;
    off_t off2 = 0;
    off_t off3 = 0;

    off1 = lseek(0, 0, SEEK_SET);    /* 0, used for stdin */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL((int32_t)off1, POSIX_FS_IS_ERROR, (int32_t)off1);

    off2 = lseek(1, 0, SEEK_SET);    /* 1, used for stdout */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL((int32_t)off2, POSIX_FS_IS_ERROR, (int32_t)off2);

    off3 = lseek(2, 0, SEEK_SET);    /* 2 used for stderr */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL((int32_t)off3, POSIX_FS_IS_ERROR, (int32_t)off3);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_LSEEK_standard
 * @tc.name     lseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
#define TEST_SEEK_WRITE_BUF "AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDD"
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsLseekStandard, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    off_t off = 0;
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };
    char writeBuf[TEST_BUF_SIZE] = { TEST_SEEK_WRITE_BUF };
    char readBuf[TEST_BUF_SIZE] = { 0 };

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = write(fd, writeBuf, TEST_BUF_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    // 头部读取入10字节
    off = lseek(fd, 0, SEEK_SET);
    ICUNIT_ASSERT_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off);

    ret = read(fd, readBuf, 10);    /* 10, common data for test, no special meaning */
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    ret = strcmp(readBuf, "AAAAAAAAAA");
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_NO_ERROR, ret, EXIT);

    // 中间读取10字节
    off = lseek(fd, 10, SEEK_CUR);  /* 10, common data for test, no special meaning */
    ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT);

    ret = read(fd, readBuf, 10);    /* 10, common data for test, no special meaning */
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    ret = strcmp(readBuf, "CCCCCCCCCC");
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_NO_ERROR, ret, EXIT);

    // 尾部向前读取10字节
    off = lseek(fd, -10, SEEK_END); /* -10, common data for test, no special meaning */
    ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT);

    ret = read(fd, readBuf, 10);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    ret = strcmp(readBuf, "DDDDDDDDDD");
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_NO_ERROR, ret, EXIT);

    // 尾部超出10字节
    off = lseek(fd, 10, SEEK_END);  /* 10, common data for test, no special meaning */
    ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT);

    ret = write(fd, 'E', 1);    /* 1, common data for test, no special meaning */
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    // 获取文件长度
    off = lseek(fd, 0, SEEK_END);
    ICUNIT_GOTO_EQUAL(off, 51, off, EXIT);  /* 51, common data for test, no special meaning */

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

void posixFsLseekTest(void)
{
    RUN_ONE_TESTCASE(testFsLseekOK001);
    RUN_ONE_TESTCASE(testFsLseekOK002);
    RUN_ONE_TESTCASE(testFsLseekOK003);
    RUN_ONE_TESTCASE(testFsLseekEBADF001);
    RUN_ONE_TESTCASE(testFsLseekEBADF002);
    RUN_ONE_TESTCASE(testFsLseekEBADF003);
    RUN_ONE_TESTCASE(testFsLseekStandard);
}

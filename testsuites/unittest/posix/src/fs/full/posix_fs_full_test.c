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

#define READ_WRITE_BUF_SIZE  1024
#define WRITE_BUF_SIZE  512

#define WRITE_BUF_LEN   128

/* *
 * @tc.number   SUB_KERNEL_FS_FULL001
 * @tc.name     open close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFull001, Function | MediumTest | Level1)
{
    int32_t fd1 = -1;
    int32_t fd2 = -1;
    int32_t ret = 0;
    const char tmpFileName[] = FILE1;

    fd1 = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd1, POSIX_FS_IS_ERROR, fd1);

#if (LOSCFG_SUPPORT_FATFS == 1)
    fd2 = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_EQUAL(fd2, POSIX_FS_IS_ERROR, fd2);
#endif

    ret = close(fd1);
    ICUNIT_ASSERT_NOT_EQUAL(fd1, POSIX_FS_IS_ERROR, fd1);

    fd2 = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd2, POSIX_FS_IS_ERROR, fd2);

    ret = close(fd2);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL002
 * @tc.name     open write read lseek read close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFull002, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    int32_t i = 0;
    off_t off = 0;
    const char tmpFileName[] = FILE1;
    char *writeBuf = (char*)malloc(READ_WRITE_BUF_SIZE * sizeof(char));
    char *readBuf = (char*)malloc(READ_WRITE_BUF_SIZE * sizeof(char));

    (void)memset_s(writeBuf, READ_WRITE_BUF_SIZE, 'w', READ_WRITE_BUF_SIZE);
    (void)memset_s(readBuf, READ_WRITE_BUF_SIZE, 'r', READ_WRITE_BUF_SIZE);

    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = write(fd, writeBuf, READ_WRITE_BUF_SIZE / 2); /* 2, common data for test, no special meaning */
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    ret = write(fd, writeBuf, READ_WRITE_BUF_SIZE / 2); /* 2, common data for test, no special meaning */
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    off = lseek(fd, 0, SEEK_SET);
    ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT);

    for (; i < 8; i++) {    /* 8, common data for test, no special meaning */
        ret = read(fd, readBuf + i * 128, 128);  /* 128, common data for test, no special meaning */
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    }

    for (int32_t i = 0; i < READ_WRITE_BUF_SIZE; i++) {
        ICUNIT_GOTO_EQUAL(writeBuf[i], readBuf[i], POSIX_FS_IS_ERROR, EXIT);
    }

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL003
 * @tc.name     mkdir opendir closedir rmdir fstat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFull003, Function | MediumTest | Level1)
{
    int32_t i = 0;
    int32_t index = 0;
    int32_t ret = 0;
    int32_t fd[16];
    DIR *dir = NULL;
    bool flag = true;
    struct dirent *dirmsg;
    struct stat buf = { 0 };
    char pathDir0[50] = TEST_ROOT"/dir0";
    char tmpFileName[50] = TEST_ROOT"/dir0/file00";
    char* fileName = "filexx";
    int32_t len = strlen(tmpFileName);

    ret = mkdir(pathDir0, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    for (; i < 16; i++) {
        tmpFileName[len - 2] = '0' + i / 10;
        tmpFileName[len - 1] = '0' + i % 10;

        fd[i] = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
        if (fd[i] == -1) {
            flag = false;
        }
        ICUNIT_GOTO_NOT_EQUAL(fd[i], POSIX_FS_IS_ERROR, fd[i], EXIT1);
    }

    dir = opendir(pathDir0);
    if (dir == NULL) {
        flag = false;
    }
    ICUNIT_GOTO_NOT_EQUAL(dir, NULL, POSIX_FS_IS_ERROR, EXIT1);

    while ((dirmsg = readdir(dir)) != NULL) {
        fileName[4] = '0' + index / 10;
        fileName[5] = '0' + index % 10;

        ret = strcmp(dirmsg->d_name, fileName);
        ICUNIT_GOTO_EQUAL(ret, POSIX_FS_NO_ERROR, ret, EXIT1);

        ret = fstat(fd[index], &buf);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
        ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT1);

        index++;
    }
    ICUNIT_GOTO_EQUAL(index, 16, POSIX_FS_IS_ERROR, EXIT1);

EXIT1:
    for (int32_t j = 0; j < i; j++) {
        tmpFileName[len - 2] = '0' + j / 10;
        tmpFileName[len - 1] = '0' + j % 10;

        ret = close(fd[j]);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

        ret = unlink(tmpFileName);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }

    if (flag == false) {
        goto EXIT;
    }

    ret = closedir(dir);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    ret = rmdir(pathDir0);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL004
 * @tc.name     mkdir readdir opendir stat closedir rmdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFull004, Function | MediumTest | Level1)
{
    int32_t i = 0;
    int32_t ret = 0;
    int32_t index = 0;
    DIR *dir = NULL;
    struct dirent *dirmsg;
    struct stat buf = { 0 };
    char *dirName = "dirxx";
    char pathDir0[50] = TEST_ROOT"/dir0";
    char tmpDirName[50] = TEST_ROOT"/dir0/dirxx";
    int32_t len = strlen(tmpDirName);

    ret = mkdir(pathDir0, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    for (; i < 16; i++) {
        tmpDirName[len - 2] = '0' + i / 10;
        tmpDirName[len - 1] = '0' + i % 10;
        ret = mkdir(tmpDirName, TEST_MODE_HIGH);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    }

    dir = opendir(pathDir0);
    ICUNIT_GOTO_NOT_EQUAL(dir, NULL, POSIX_FS_IS_ERROR, EXIT1);

    while ((dirmsg = readdir(dir)) != NULL) {
        dirName[3] = '0' + index / 10;
        dirName[4] = '0' + index % 10;
        ret = strcmp(dirmsg->d_name, dirName);
        ICUNIT_GOTO_EQUAL(ret, POSIX_FS_NO_ERROR, ret, EXIT1);

        tmpDirName[len - 2] = '0' + index / 10;
        tmpDirName[len - 1] = '0' + index % 10;
        ret = stat(tmpDirName, &buf);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
        ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFDIR, POSIX_FS_IS_ERROR, EXIT1);

        index++;
    }
    ICUNIT_GOTO_EQUAL(index, 16, POSIX_FS_IS_ERROR, EXIT1);

EXIT1:
    ret = closedir(dir);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    for (int32_t j = 0; j < i; j++) {
        tmpDirName[len - 2] = '0' + j / 10;
        tmpDirName[len - 1] = '0' + j % 10;
        ret = rmdir(tmpDirName);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }

    ret = rmdir(pathDir0);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL005
 * @tc.name     read write lseek close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFull005, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    bool flag = true;
    off_t off = 0;
    const char tmpFileName[]= FILE1;
    char *writeBuf = (char*)malloc(WRITE_BUF_LEN * sizeof(char));
    char readBuf = 'r';

    (void)memset_s(writeBuf, WRITE_BUF_LEN, 'w', WRITE_BUF_LEN);

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = write(fd, writeBuf, WRITE_BUF_LEN);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    off = lseek(fd, 0, SEEK_SET);
    ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT);

    for (int i = 1; i <= 16; i++) {
        ret = read(fd, &readBuf, 1);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
        if (readBuf != 'w') {
            flag = false;
            break;
        }
        readBuf = 'r';
        off = lseek(fd, 7, SEEK_CUR);
        ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT);
    }

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL006
 * @tc.name     open fstat close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFull006, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    struct stat buf = { 0 };
    const char tmpFileName[] = FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = fstat(fd, &buf);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT);

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL007
 * @tc.name     open stat close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFull007, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    struct stat buf = { 0 };
    const char tmpFileName[] = FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = stat(tmpFileName, &buf);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT);

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = stat(tmpFileName, &buf);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL008
 * @tc.name     mkdir stat rmdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFull008, Function | MediumTest | Level1)
{
    int32_t ret = 0;
    char pathDir0[50] = TEST_ROOT"/dir0";
    struct stat buf = { 0 };

    ret = mkdir(pathDir0, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = stat(pathDir0, &buf);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
    ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFDIR, POSIX_FS_IS_ERROR, EXIT1);

    ret = rmdir(pathDir0);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);

    ret = stat(pathDir0, &buf);
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    return POSIX_FS_NO_ERROR;

EXIT1:
    ret = rmdir(pathDir0);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    return POSIX_FS_IS_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL009
 * @tc.name     open close stat rename unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFull009, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    struct stat buf = { 0 };
    char tmpFileName1[] = FILE7;
    char tmpFileName2[] = FILE8;

    fd = open(tmpFileName1, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = close(fd);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);

    ret = stat(tmpFileName1, &buf);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT);

    ret = rename(tmpFileName1, tmpFileName2);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    ret = stat(tmpFileName1, &buf);
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    ret = stat(tmpFileName2, &buf);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT);

    ret = unlink(tmpFileName1);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName2);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;

EXIT1:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    ret = unlink(tmpFileName1);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

// /* *
//  * @tc.number   SUB_KERNEL_FS_FULL010
//  * @tc.name     open write stat ftruncate close unlink
//  * @tc.desc     [C- SOFTWARE -0200]
//  */
// LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFull010, Function | MediumTest | Level1)
// {
//     struct stat buf = { 0 };
//     int32_t fd = -1;
//     int32_t ret = 0;
//     unsigned char test[64] = { 0 };
//     char tmpFileName[TEST_BUF_SIZE] = FILE3;
//     char *writeBuf = (char*)malloc(WRITE_BUF_SIZE * sizeof(char));

//     (void)memset_s(writeBuf, WRITE_BUF_SIZE, 'w', WRITE_BUF_SIZE);

//     fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
//     ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

//     ret = write(fd, writeBuf, WRITE_BUF_SIZE);
//     ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

//     ret = stat(tmpFileName, &buf);
//     ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
//     ICUNIT_GOTO_EQUAL(buf.st_size, WRITE_BUF_SIZE, POSIX_FS_IS_ERROR, EXIT);

//     ret = ftruncate(fd, WRITE_BUF_SIZE / 2);
//     ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

//     ret = stat(tmpFileName, &buf);
//     ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
//     ICUNIT_GOTO_EQUAL(buf.st_size, WRITE_BUF_SIZE / 2, POSIX_FS_IS_ERROR, EXIT);

//     ret = ftruncate(fd, WRITE_BUF_SIZE);
//     ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

//     ret = stat(tmpFileName, &buf);
//     ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
//     ICUNIT_GOTO_EQUAL(buf.st_size, WRITE_BUF_SIZE, POSIX_FS_IS_ERROR, EXIT);

// EXIT:
//     ret = close(fd);
//     ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

//     ret = unlink(tmpFileName);
//     ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

//     return POSIX_FS_NO_ERROR;
// }

void posixFsFullTest(void)
{
    RUN_ONE_TESTCASE(testFsFull001);
    RUN_ONE_TESTCASE(testFsFull002);
    RUN_ONE_TESTCASE(testFsFull003);
    RUN_ONE_TESTCASE(testFsFull004);
    RUN_ONE_TESTCASE(testFsFull005);
    RUN_ONE_TESTCASE(testFsFull006);
    RUN_ONE_TESTCASE(testFsFull007);
    RUN_ONE_TESTCASE(testFsFull008);
    RUN_ONE_TESTCASE(testFsFull009);
    // RUN_ONE_TESTCASE(testFsFull010);
}

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
#include "limits.h"

#define MAX_OPEN_FILES_NUM          (NR_OPEN_DEFAULT - 3)
#define MAX_FILE_NAME_LEN           60

#define STRESS_RUN_TIMES            1      // >= 100
#define MAX_OPEN_DIRS_NUM           16      // MAX_OPEN_DIRS

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsSTRESS001, Function | MediumTest | Level1)
{
    int32_t i = 0;  // open file index
    int32_t j = 0;  // mkdir/opendir dir index
    int32_t ret = 0;
    int32_t ret1 = 0;
    int32_t ret2 = 0;
    bool flagOpen = true;
    bool flagOpendir = true;
    int32_t fd[MAX_OPEN_FILES_NUM];
    DIR *dir[MAX_OPEN_DIRS_NUM] = { 0 };
    char tmpFileName[] = TEST_ROOT"/aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeeeeee";
    int32_t lenFileName = strlen(tmpFileName);
    char tmpDirName[] = TEST_ROOT"/gggggggggg";
    int32_t lenDirName = strlen(tmpDirName);

    // unlink 文件
    for (int32_t index = 0; index < MAX_OPEN_FILES_NUM; index++) {
        tmpFileName[lenFileName - 2] = '0' + index / 10;
        tmpFileName[lenFileName - 1] = '0' + index % 10;
        (void)unlink(tmpFileName);
        fd[index] = -1;
    }
    // rmdir 目录
    for (int32_t index = 0; index < MAX_OPEN_DIRS_NUM; index++) {
        tmpDirName[lenDirName - 2] = '0' + index / 10;
        tmpDirName[lenDirName - 1] = '0' + index % 10;
        (void)rmdir(tmpDirName);
    }

    int32_t flagTestFailedTimes = 0;
    int32_t flagOpenFailedTimes[STRESS_RUN_TIMES];
    int32_t flagMkdirFailedTimes[STRESS_RUN_TIMES];
    int32_t flagCloseUnlinkFailedTimes[STRESS_RUN_TIMES];
    int32_t flagClosedirRmdirFailedTimes[STRESS_RUN_TIMES];

    for (int32_t times = 0; times < STRESS_RUN_TIMES; times++) {
        // 1 open 文件
        i = 0;
        flagOpenFailedTimes[times] = 0;
        for (; i < MAX_OPEN_FILES_NUM; i++) {
            tmpFileName[lenFileName - 2] = '0' + i / 10;
            tmpFileName[lenFileName - 1] = '0' + i % 10;
            fd[i] = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
            if (fd[i] == -1) {
                flagOpenFailedTimes[times]++;
            }
        }
        // 2 mkdir 目录
        j = 0;
        flagMkdirFailedTimes[times] = 0;
        for (; j < MAX_OPEN_DIRS_NUM; j++) {
            tmpDirName[lenDirName - 2] = '0' + j / 10;
            tmpDirName[lenDirName - 1] = '0' + j % 10;
            ret = mkdir(tmpDirName, TEST_MODE_HIGH);
            dir[j] = opendir(tmpDirName);
            if (ret == -1 || dir[j] == NULL) {
                flagMkdirFailedTimes[times]++;
            }
        }
        // 3 close unlink 文件
        flagCloseUnlinkFailedTimes[times] = 0;
        for (int32_t m = 0; m < i; m++) {
            tmpFileName[lenFileName - 2] = '0' + m / 10;
            tmpFileName[lenFileName - 1] = '0' + m % 10;
            ret1 = close(fd[m]);
            ret2 = unlink(tmpFileName);
            if (ret1 == -1 || ret2 == -1) {
                flagCloseUnlinkFailedTimes[times]++;
            }
        }
        // 4 rmdir 目录
        flagClosedirRmdirFailedTimes[times] = 0;
        for (int32_t n = 0; n < j; n++) {
            tmpDirName[lenDirName - 2] = '0' + n / 10;
            tmpDirName[lenDirName - 1] = '0' + n % 10;
            ret1 = closedir(dir[n]);
            ret2 = rmdir(tmpDirName);
            if (ret1 == -1 || ret2 == -1) {
                flagClosedirRmdirFailedTimes[times]++;
            }
        }
        if (flagOpenFailedTimes[times] != 0 || flagMkdirFailedTimes[times] != 0 ||
            flagCloseUnlinkFailedTimes[times] != 0 || flagClosedirRmdirFailedTimes[times] != 0) {
            flagTestFailedTimes++;
        }
    }

    printf("STRESS_RUN_TIMES = %d, OK = %d, NOK = %d\n\r", STRESS_RUN_TIMES,
                                                           STRESS_RUN_TIMES - flagTestFailedTimes,
                                                           flagTestFailedTimes);
    ICUNIT_ASSERT_EQUAL(flagTestFailedTimes, POSIX_FS_NO_ERROR, POSIX_FS_IS_ERROR);
    return POSIX_FS_NO_ERROR;

// EXIT:
//     //  close unlink 文件
//     for (int32_t index = 0; index < i; index++) {   /* open failed */
//         tmpFileName[lenFileName - 2] = '0' + index / 10;
//         tmpFileName[lenFileName - 1] = '0' + index % 10;

//         ret = close(fd[index]);
//         ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

//         ret = unlink(tmpFileName);
//         ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
//     }

//     if (flagOpen == false) {
//         goto EXIT1;
//     }

//     for (int32_t index = 0; index < j; index++) {   /* mkdir failed */
//         tmpDirName[lenDirName - 2] = '0' + index / 10;
//         tmpDirName[lenDirName - 1] = '0' + index % 10;

//         ret = closedir(dir[index]);
//         ICUNIT_ASSERT_NOT_EQUAL(ret , POSIX_FS_IS_ERROR, ret);

//         ret = rmdir(tmpDirName);
//         ICUNIT_ASSERT_NOT_EQUAL(ret , POSIX_FS_IS_ERROR, ret);
//     }

//     if (flagOpendir == false) {     /* opendir failed */
//         tmpDirName[lenDirName - 2] = '0' + j / 10;
//         tmpDirName[lenDirName - 1] = '0' + j % 10;

//         ret = rmdir(tmpDirName);
//         ICUNIT_ASSERT_NOT_EQUAL(ret , POSIX_FS_IS_ERROR, ret);
//     }
// EXIT1:
//     return POSIX_FS_NO_ERROR;
}

#define READ_WRITE_BUF_SIZE  1024
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsSTRESS002, Function | MediumTest | Level1)
{
    int32_t fd = -1;
    int32_t ret = 0;
    off_t off = 0;
    bool flag = true;
    const char tmpFileName[] = FILE1;
    char *writeBuf = (char*)malloc(READ_WRITE_BUF_SIZE * sizeof(char));
    char *readBuf = (char*)malloc(READ_WRITE_BUF_SIZE * sizeof(char));

    (void)memset_s(writeBuf, READ_WRITE_BUF_SIZE, 'w', READ_WRITE_BUF_SIZE);

    for (int32_t times = 0; times < STRESS_RUN_TIMES; times++) {
        (void)memset_s(readBuf, READ_WRITE_BUF_SIZE, 'r', READ_WRITE_BUF_SIZE);

        fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
        ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

        ret = write(fd, writeBuf, READ_WRITE_BUF_SIZE / 2);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
        ret = write(fd, writeBuf, READ_WRITE_BUF_SIZE / 2);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);

        off = lseek(fd, 0, SEEK_SET);
        ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT1);

        for (int32_t i = 0; i < 8; i++) {
            ret = read(fd, readBuf + i * 128, 128);
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
        }

        for (int32_t i = 0; i < READ_WRITE_BUF_SIZE; i++) {
            ICUNIT_GOTO_EQUAL(writeBuf[i], readBuf[i], POSIX_FS_IS_ERROR, EXIT1);
        }

        ret = close(fd);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);

        ret = unlink(tmpFileName);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    }

    return POSIX_FS_NO_ERROR;

EXIT1:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

LITE_TEST_CASE(PosixFsFuncTestSuite, testFsSTRESS003, Function | MediumTest | Level1)
{
    printf("testFsSTRESS003 PATH_MAX = %d\n\r", PATH_MAX);
    errno = 0;
    int32_t fd = -1;
    int32_t ret = 0;
    // char *tmpFileName = (char*)malloc(PATH_MAX * sizeof(char));
    char tmpFileName[PATH_MAX];

    (void)memset_s(tmpFileName, PATH_MAX, 0, PATH_MAX);
    int32_t len = strlen(TEST_ROOT);
    printf("len = %d, TEST_ROOT = %s\n\r", len, TEST_ROOT);
    for (int32_t i = 0; i < PATH_MAX; i++) {
        if (i < len) {
            tmpFileName[i] = TEST_ROOT[i];
        } else if (i == len) {
            tmpFileName[i] = '/';
        } else {
            tmpFileName[i] = 'f';
        }
    }

    printf("tmpFileName's len = %d\n\r", strlen(tmpFileName));
    printf("tmpFileName = ");
    for (int32_t i = 0; i < PATH_MAX; i++) {
        printf("%c", tmpFileName[i]);
    }
    printf("\n\r");
    for (int32_t times = 0; times < STRESS_RUN_TIMES; times++) {
        fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
        printf("errno = %d\n\r");
        ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

        ret = close(fd);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);

        ret = unlink(tmpFileName);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    }

    return POSIX_FS_NO_ERROR;

EXIT1:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

#define DIR_NUM 2048
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsSTRESS004, Function | MediumTest | Level1)
{
    int32_t i = 0;
    int32_t ret = 0;
    DIR *dir[DIR_NUM] = { 0 };
    char *dirName = TEST_ROOT"/dirxxxx";
    int32_t len = strlen(dirName);

    for (; i < DIR_NUM; i++) {
        dirName[len - 4] = '0' + i / 1000;
        dirName[len - 3] = '0' + (i / 100) % 10;
        dirName[len - 2] = '0' + (i / 10) % 10;
        dirName[len - 1] = '0' + i % 10;
        ret = mkdir(dirName, TEST_MODE_HIGH);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    }
EXIT:
    for (int32_t j = 0; j < i; j++) {
        dirName[len - 4] = '0' + j / 1000;
        dirName[len - 3] = '0' + (j / 100) % 10;
        dirName[len - 2] = '0' + (j / 10) % 10;
        dirName[len - 1] = '0' + j % 10;
        ret = rmdir(dirName);
        ICUNIT_ASSERT_NOT_EQUAL(ret , POSIX_FS_IS_ERROR, ret);
    }
    return POSIX_FS_NO_ERROR;
}

#define MAX_OPEN_FILE_NUM 16
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsSTRESS005, Function | MediumTest | Level1)
{
    int32_t j = 0;  // open
    int32_t k = 0;  // mkdir
    int32_t ret = 0;
    bool flagMkdir = true;
    struct stat buf = { 0 };
    int32_t fd[MAX_OPEN_FILE_NUM];
    char *tmpFileName = TEST_ROOT"/filexx";
    char *tmpDirName = TEST_ROOT"/dirxx";
    int32_t len = strlen(tmpFileName);

    for (int32_t times = 0; times < STRESS_RUN_TIMES; times++) {
        // 打开创建文件 0-15
        j = 0;
        for (; j < MAX_OPEN_FILE_NUM; j++) {
            tmpFileName[len - 2] = '0' + j / 10;
            tmpFileName[len - 1] = '0' + j % 10;

            fd[j] = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
            ICUNIT_GOTO_NOT_EQUAL(fd[j], POSIX_FS_IS_ERROR, fd[j], EXIT3);
        }
        // fstat文件 0-15
        for (int32_t i = 0; i < MAX_OPEN_FILE_NUM; i++) {
            ret = fstat(fd[i], &buf);
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT3);
            // 比较文件属性
            ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT3);
        }
        // 关闭文件 0-15
        for (int32_t i = 0; i < MAX_OPEN_FILE_NUM; i++) {
            ret = close(fd[i]);
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT2);
        }
        // stat文件 0-15
        for (int32_t i = 0; i < MAX_OPEN_FILE_NUM; i++) {
            tmpFileName[len - 2] = '0' + i / 10;
            tmpFileName[len - 1] = '0' + i % 10;

            ret = stat(tmpFileName, &buf);
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
            // 比较属性 0-15
            ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT1);
        }
        // 创建目录 0-15
        k = 0;
        for (; k < MAX_OPEN_DIRS_NUM; k++) {
            tmpDirName[len - 2] = '0' + k / 10;
            tmpDirName[len - 1] = '0' + k % 10;

            ret = mkdir(tmpDirName, TEST_MODE_HIGH);
            if (ret == POSIX_FS_IS_ERROR) {
                flagMkdir = false;
            }
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
        }
        // stat目录 0-15
        for (int32_t i = 0; i < MAX_OPEN_DIRS_NUM; i++) {
            tmpDirName[len - 2] = '0' + i / 10;
            tmpDirName[len - 1] = '0' + i % 10;

            ret = stat(tmpDirName, &buf);
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
            // 比较属性 0-15
            ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFDIR, POSIX_FS_IS_ERROR, EXIT1);
        }
        // 删除文件 0-15
        for (int32_t i = 0; i < MAX_OPEN_FILE_NUM; i++) {
            tmpFileName[len - 2] = '0' + i / 10;
            tmpFileName[len - 1] = '0' + i % 10;

            ret = unlink(tmpFileName);
            ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
        }
        // 删除目录 0-15
        for (int32_t i = 0; i < MAX_OPEN_DIRS_NUM; i++) {
            tmpDirName[len - 2] = '0' + i / 10;
            tmpDirName[len - 1] = '0' + i % 10;

            ret = rmdir(tmpDirName);
            ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
        }
        // stat 文件 0-15 失败
        for (int32_t i = 0; i < MAX_OPEN_FILE_NUM; i++) {
            tmpFileName[len - 2] = '0' + i / 10;
            tmpFileName[len - 1] = '0' + i % 10;

            ret = stat(tmpFileName, &buf);
            ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
        }
        // stat 目录 0-15 失败
        for (int32_t i = 0; i < MAX_OPEN_DIRS_NUM; i++) {
            tmpDirName[len - 2] = '0' + i / 10;
            tmpDirName[len - 1] = '0' + i % 10;

            ret = stat(tmpDirName, &buf);
            ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
        }
    }

    return POSIX_FS_NO_ERROR;

EXIT3:
    for (int32_t m = 0; m < j; m++) {
        ret = close(fd[m]);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }
    goto EXIT1;
EXIT2:
    for (int32_t m = j; m < MAX_OPEN_FILE_NUM; m++) {
        ret = close(fd[m]);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }
EXIT1:
    for (int32_t m = 0; m < j; m++) {
        tmpFileName[len - 2] = '0' + m / 10;
        tmpFileName[len - 1] = '0' + m % 10;

        ret = unlink(tmpFileName);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }
    if (flagMkdir == false) {
        goto EXIT;
    }
    return POSIX_FS_NO_ERROR;
EXIT:
    for (int32_t m = 0; m < k; m++) {
        tmpDirName[len - 2] = '0' + m / 10;
        tmpDirName[len - 1] = '0' + m % 10;
        ret = rmdir(tmpDirName);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }
    return POSIX_FS_NO_ERROR;
}

void posixFsStressTest(void)
{
    // RUN_ONE_TESTCASE(testFsSTRESS001);
    // RUN_ONE_TESTCASE(testFsSTRESS002);
    // RUN_ONE_TESTCASE(testFsSTRESS003);
    // RUN_ONE_TESTCASE(testFsSTRESS004);
    // RUN_ONE_TESTCASE(testFsSTRESS005);
}

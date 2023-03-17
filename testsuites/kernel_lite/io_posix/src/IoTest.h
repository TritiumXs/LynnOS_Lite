/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef KERNEL_LITE_IO_TEST_H
#define KERNEL_LITE_IO_TEST_H

#include <stdio.h>
#include "log.h"
#include "utils.h"

#define IOTEST_TEMPFILE "/littlefs/io_posix_test.txt"

#define INIT_TEST_FILE(fp) do {                              \
    FOPEN_WRITE(fp);                                         \
    TEST_ASSERT_NOT_EQUAL(fputs("hello world", fp), -1)      \
    TEST_ASSERT_NOT_EQUAL(fclose(fp), -1);                   \
} while (0)

#define FOPEN_WRITE(fp) do {                                 \
    fp = fopen(IOTEST_TEMPFILE, "w");                        \
    TEST_ASSERT_NOT_EQUAL(NULL, fp);                         \
} while (0)

#define FOPEN_READ(fp) do {                                  \
    fp = fopen(IOTEST_TEMPFILE, "r");                        \
    TEST_ASSERT_NOT_EQUAL(fp, NULL);                         \
} while (0)

#define FILENO(fp) do {                                      \
    fd = fileno(fp);                                         \
    TEST_ASSERT_NOT_EQUAL(fd, -1);                           \
} while (0)

#endif

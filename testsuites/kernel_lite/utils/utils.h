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

#ifndef KERNEL_LITE_UTILS
#define KERNEL_LITE_UTILS

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <pthread.h>

#define RUN_ONE_TESTCASE(caseName) UnityDefaultTestRun(caseName##_runTest, __FILE__, __LINE__)

#define LogPrint(...)              LOG(__VA_ARGS__)

#define EXPECT_EQ(retCode, value)      TEST_ASSERT_EQUAL_INT(value, retCode)            // retCode = value
#define EXPECT_NE(retCode, value)      TEST_ASSERT_NOT_EQUAL(value, retCode)            // retCode != value
#define EXPECT_GT(retCode, value)      TEST_ASSERT_GREATER_THAN_INT(value, retCode)     // retCode > value
#define EXPECT_LT(retCode, value)      TEST_ASSERT_LESS_THAN_INT(value, retCode)        // retCode < value
#define EXPECT_GE(retCode, value)      TEST_ASSERT_GREATER_OR_EQUAL_INT(value, retCode) // retCode >= value
#define EXPECT_LE(retCode, value)      TEST_ASSERT_LESS_THAN_INT(value, retCode)        // retCode <= value

#define ASSERT_FALSE(retCode)          TEST_ASSERT_FALSE(retCode)                       // retCode = flash
#define ASSERT_TRUE(retCode)           TEST_ASSERT_TRUE(retCode)                        // retCode = true, 致命的失败
#define EXPECT_FALSE(retCode)          TEST_ASSERT_FALSE(retCode)
#define EXPECT_TRUE(retCode)           TEST_ASSERT_TRUE(retCode)                        // retCode = true, 非致命的失败
#define EXPECT_STREQ(retCode, value)   TEST_ASSERT_EQUAL_STRING(value, retCode)

#define ASSERT_EQ(retCode, value)      TEST_ASSERT_EQUAL_INT(value, retCode)
#define EXPECT_STRNE(a, b)                                                              // not equal
#define ASSERT_NE(retCode, value)      TEST_ASSERT_NOT_EQUAL(value, retCode)
#define FAIL(message)                  printf("%s:%s:\n"message,__func__,__LINE__)      // TEST_FAIL_MESSAGE(message)
#define nullptr                        NULL
#define PAGE_SIZE                      (0x1000)

#define EXPECT_FLOAT_EQ(retCode, value)        TEST_ASSERT_EQUAL_FLOAT(value, retCode);
#define EXPECT_DOUBLE_EQ(retCode, value)       TEST_ASSERT_EQUAL_DOUBLE(value, retCode);

#define ADD_FAILURE() printf("%s:%s:ADD_FAILURE Execution failed!\n",__func__,__LINE__); // GTEST_NONFATAL_FAILURE_("Failed")
#define assert        printf("enter assert\n");while(1);

// get thread id
#define gettid() ((pid_t)syscall(SYS_gettid))

// check if 'actual' is close to 'target', within 5% in default
int CheckValueClose(double target, double actual, double accuracy);

// sleep n millisecond(1/1000 sec)
void Msleep(int n);

/**
 * desc:   check process state use 'waitpid'
 * input:  pid  -- target pid
 *         code -- store exit code or signal number
 *         flag -- flag for waitpid, default to WNOHANG
 * output: -1 -- waitpid return -1
 *         -2 -- waitpid return value error(not -1 and not pid)
 *          0 -- target process still alive
 *          1 -- target process existd, exist code is set in 'code'
 *          2 -- target process killed by a signal, signal number is set in 'code'
 *          3 -- target process is stopped,  signal number is set in 'code'
 *          4 -- get target process state error, due to waitpid error
 */
int CheckProcStatus(pid_t pid, int* code, int flag);

// make sure process is still alive
#define AssertProcAlive(pid)  do {                                        \
        int exitCode;                                                     \
        int procStat = CheckProcStatus(pid, &exitCode, WNOHANG);          \
        TEST_ASSERT_EQUAL_INT(procStat, 0);                               \
    } while (0)
#define ExpectProcAlive(pid)  do {                                        \
        int exitCode;                                                     \
        int procStat = CheckProcStatus(pid, &exitCode, WNOHANG);          \
        TEST_ASSERT_EQUAL_INT(procStat, 0);                               \
    } while (0)

// make sure process exited with exitCode 0
#define AssertProcExitedOK(pid)  do {                                  \
        int exitCode;                                                  \
        int procStat = CheckProcStatus(pid, &exitCode, WNOHANG);       \
        TEST_ASSERT_EQUAL_INT(procStat, 1);                            \
        TEST_ASSERT_EQUAL_INT(exitCode, 0);                            \
    } while (0)
#define ExpectProcExitedOK(pid)  do {                                  \
        int exitCode;                                                  \
        int procStat = CheckProcStatus(pid, &exitCode, WNOHANG);       \
        TEST_ASSERT_EQUAL_INT(procStat, 1);                            \
        TEST_ASSERT_EQUAL_INT(exitCode, 0);                            \
    } while (0)
// wait until child statu changed
#define WaitProcExitedOK(pid)  do {                                    \
        int exitCode;                                                  \
        int procStat = CheckProcStatus(pid, &exitCode, 0);             \
        TEST_ASSERT_EQUAL_INT(procStat, 1);                            \
        TEST_ASSERT_EQUAL_INT(exitCode, 0);                            \
    } while (0)

// make sure process killed by signal signum
#define AssertProcKilled(pid, signum)  do {                                          \
        int exitCode;                                                                \
        int procStat = CheckProcStatus(pid, &exitCode, WNOHANG);                     \
        TEST_ASSERT_EQUAL_INT(procStat, 2);                                          \
        TEST_ASSERT_EQUAL_INT(exitCode, signum);                                     \
    } while (0)
#define ExpectProcKilled(pid, signum)  do {                                          \
        int exitCode;                                                                \
        int procStat = CheckProcStatus(pid, &exitCode, WNOHANG);                     \
        TEST_ASSERT_EQUAL_INT(procStat, 2);                                          \
        TEST_ASSERT_EQUAL_INT(exitCode, signum);                                     \
    } while (0)
// wait until child statu changed
#define WaitProcKilled(pid, signum)  do {                                            \
        int exitCode;                                                                \
        int procStat = CheckProcStatus(pid, &exitCode, 0);                           \
        TEST_ASSERT_EQUAL_INT(procStat, 2);                                          \
        TEST_ASSERT_EQUAL_INT(exitCode, signum);                                     \
    } while (0)

// for now, crash process act like killed by SIGUSR2
#define ExpectProcCrashed(pid) WaitProcKilled(pid, SIGUSR2)

// keep current process run for a specific time, no sleep.
// msec is millisecond (1/1000 sec).
// return value is the loop count(generally you don't need to care about it)
int KeepRun(int msec);

/**
 * code to determain if execve is faild, may confic with actual sub program's exit code
 */
static const int EXECVE_RETURN_ERROR = 190;  // execve return -1
static const int EXECVE_RETURN_OK    = 191;  // execve return not -1: execve should never return on success

/**
 * desc:   start a elf, check if execve success, and return child process exit code within timeout_sec
 * input:  fname, argv, envp -- parameters for execve
 *         timeout_sec       -- timeout of the child executing, default: 5 seconds.
 *                              timeout_sec<=0 means no timeout, wait forever until child exit.
 * output: -1 -- something wrong for internal syscall, check log for detail
 *         -2 -- child does not finish in 'timeout_sec'
 *     n(>=0) -- child exit code
 */
int RunElf(const char* fname, char* const argv[], char* const envp[], int timeoutSec);

/**
 * desc:   call execve with error parameters(e.g. a non-exist file)
 * input:  fname, argv, envp -- parameters for execve
 * output:  0 -- execve ok
 *         -1 -- unexpected fork error
 *         -2 -- unexpected execve error
 *         -3 -- unknow error
 *      n(>0) -- errno of execve
 */
int StartExecveError(const char* fname, char* const argv[], char* const envp[]);

// Get a pid number that currently not exist
pid_t GetNonExistPid();

/**
 * return random number n: 0 < n <= max
 * we found in most case '0' is not expected.
 */
uint32_t GetRandom(uint32_t max);

/**
 * desc:    get current time, plus 'ms'
 */
void GetDelayedTime(struct timespec *ts, unsigned int ms);

/**
 * desc:    calculate time difference, in ms
 * output:  return time difference, unit is ms
 */
int GetTimeDiff(struct timespec ts1, struct timespec ts2);

/**
 * desc:    fix calling process to one cpu
 * output:  return 0 successful, -1 fail
 */
// int FixCurProcessToOneCpu(int cpuIndex, cpu_set_t* pOldMask);

/**
 * desc:    get cpu count
 * output:  return cpu count
 */
int GetCpuCount(void);
uint32_t GetRandom(uint32_t max);

#endif
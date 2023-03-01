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

// utils for multi-thread and multi-process test

#include "mt_utils.h"
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "log.h"

#define MAX_INDEX 12
const uint32_t MSECOND_INDEX[MAX_INDEX] = {10, 20, 30, 60, 100, 150, 200, 300, 400, 600, 1000, 2000};
const uint32_t NUMBER_INDEX[MAX_INDEX]  = {2500, 3600, 4500, 6600, 8800, 1000, 12000, 15000, 18000, 2000, 3000, 42000 };

int CountPrimes(uint32_t maxNumber)
{
    uint32_t primesCount = 0;
    uint32_t i, num;
    for (num = 2; num <= maxNumber; ++num) {
        for (i = 2; i <= num; i++) {
            if (num % i == 0) {
                break;
            }
        }
        if (i == num) {
            primesCount++;
        }
    }
    return primesCount;
}

static uint32_t GetNumberFromMS(uint32_t ms)
{
    for (uint32_t i = 0; i < MAX_INDEX; i++) {
        if (MSECOND_INDEX[i] == ms) {
            return NUMBER_INDEX[i];
        }
    }
    return 0;
}

int BusyRun(uint32_t ms)
{
    uint32_t n = GetNumberFromMS(ms);
    if (n == 0) {
        LOG("BusyRun Error: %d ms not support", n);
        _exit(1);
    }
    return CountPrimes(n);
}

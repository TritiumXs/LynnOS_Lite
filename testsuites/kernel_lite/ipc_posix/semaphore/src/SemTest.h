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

#ifndef SEM_TEST_H
#define SEM_TEST_H

#define NANO_S 1000000000
#define NANO_MS 1000000

#define TEN_CONT 10
#define FIFTY_CONT 50
#define HUNDRED_CONT 100

#define HIGHEST_USER_PROCESS_PRIORITY 10
#define LOWEST_USER_PROCESS_PRIORITY 31
#define HIGHEST_USER_THREAD_PRIORITY 0
#define LOWEST_USER_THREAD_PRIORITY 31

#define DEFAULT_SHELL_PROCESS_PRIORITY 15
#define DEFAULT_INIT_PROCESS_PRIORITY 28
#define DEFAULT_KERNEL_PROCESS_PRIORITY 0

#define DEFAULT_THREAD_PRIORITY 25

#define DEFAULT_RR_SCHED_INTERVAL 20000000 // defaluit sched interval of RR, in ms

#endif /* SEM_TEST_H */

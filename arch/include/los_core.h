/*
* Copyright (c) 2023 Hunan OpenValley Digital Industry Development Co., Ltd.
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

#ifndef _LOS_CORE_H
#define _LOS_CORE_H

#include "los_config.h"
#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOSCFG_KERNEL_SMP
struct SmpOps {
    INT32 (*SmpCpuOn)(UINT32 cpuNum, UINTPTR startEntry);  /* The startEntry is physical addr. */
};

typedef VOID (*ArchCoreStartFunc)(VOID);

VOID ArchCpuOn(UINT32 cpuNum, ArchCoreStartFunc func, struct SmpOps *ops, VOID *arg);
UINT32 ArchCurrCpuid(VOID);
#else
STATIC INLINE UINT32 ArchCurrCpuid(VOID)
{
    return 0;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
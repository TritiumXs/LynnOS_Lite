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

#ifndef _LOS_SPINLOCK_H
#define _LOS_SPINLOCK_H

#include "los_config.h"
#include "los_compiler.h"
#include "los_interrupt.h"

#include <stdlib.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

VOID ArchSpinLock(UINT32 *lock);
VOID ArchSpinUnlock(UINT32 *lock);

typedef struct {
    UINT32 rawLock;
} Spinlock;

#define SPIN_LOCK_INITIALIZER(lockName) \
{                                       \
    .rawLock    = 0U,                   \
}

#ifdef LOSCFG_KERNEL_SMP

STATIC INLINE VOID LOS_SpinInit(Spinlock *lock)
{
    lock->rawLock = 0;
}

STATIC INLINE VOID LOS_SpinLock(Spinlock *lock)
{
    ArchSpinLock(&lock->rawLock);
}

STATIC INLINE VOID LOS_SpinUnlock(Spinlock *lock)
{
    ArchSpinUnlock(&lock->rawLock);
}

STATIC INLINE VOID LOS_SpinLockSave(Spinlock *lock, UINT32 *intSave)
{
    LOS_ASSERT(lock);

    *intSave = ArchIntLock();
    LOS_SpinLock(lock);
}

STATIC INLINE VOID LOS_SpinUnlockRestore(Spinlock *lock, UINT32 intSave)
{
    LOS_ASSERT(lock);

    LOS_SpinUnlock(lock);
    ArchIntRestore(intSave);
}

#else

STATIC INLINE VOID LOS_SpinInit(Spinlock *lock)
{
    lock->rawLock = 0;
}

STATIC INLINE VOID LOS_SpinLock(Spinlock *lock)
{
    (VOID)lock;
}

STATIC INLINE VOID LOS_SpinUnlock(Spinlock *lock)
{
    (VOID)lock;
}

STATIC INLINE VOID LOS_SpinLockSave(Spinlock *lock, UINT32 *intSave)
{
    (VOID)lock;
    *intSave = ArchIntLock();
}

STATIC INLINE VOID LOS_SpinUnlockRestore(Spinlock *lock, UINT32 intSave)
{
    (VOID)lock;
    ArchIntRestore(intSave);
}
#endif

#define SPIN_LOCK_INIT(lock)  Spinlock lock = SPIN_LOCK_INITIALIZER(lock)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

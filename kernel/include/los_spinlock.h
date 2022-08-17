/*
 * Copyright (c) 2022 Hunan OpenValley Digital Industry Development Co., Ltd.
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

#include "los_interrupt.h"
#include "los_atomic.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define LOS_SPINLOCK_FREE           (0)
#define LOS_SPINLOCK_LOCK           (1)
#define LOS_SPINLOCK_INITIALIZER    { .locked = LOS_SPINLOCK_FREE }

struct Spinlock {
    Atomic locked; /**< 0: free, 1: locked */
};

STATIC INLINE VOID LOS_SpinLock(struct Spinlock *lock)
{
    (VOID)lock;

#if LOSCFG_KERNEL_SMP
    while (LOS_AtomicCmpXchg32bits(&(lock->locked), LOS_SPINLOCK_LOCK, LOS_SPINLOCK_FREE) != TRUE) {
    }
#endif
}

STATIC INLINE INT32 LOS_SpinTrylock(struct Spinlock *lock)
{
    INT32 ret = LOS_OK;

    (VOID)lock;

#if LOSCFG_KERNEL_SMP
    ret = ((LOS_AtomicCmpXchg32bits(&(lock->locked), LOS_SPINLOCK_LOCK, LOS_SPINLOCK_FREE) != TRUE)?(LOS_OK):(LOS_NOK));
#endif

    return ret;
}

STATIC INLINE VOID LOS_SpinUnlock(struct Spinlock *lock)
{
    (VOID)lock;

#if LOSCFG_KERNEL_SMP
    LOS_AtomicSet(&(lock->locked), LOS_SPINLOCK_FREE);
#endif
}

STATIC INLINE VOID LOS_SpinLockSave(struct Spinlock *lock, UINT32 *intSave)
{
    (VOID)lock;

    *intSave = LOS_IntLock();

#if LOSCFG_KERNEL_SMP
    LOS_SpinLock(lock);
#endif
}

STATIC INLINE VOID LOS_SpinUnlockRestore(struct Spinlock *lock, UINT32 intSave)
{
    (VOID)lock;

#if LOSCFG_KERNEL_SMP
    LOS_SpinUnlock(lock);
#endif

    LOS_IntRestore(intSave);
}

STATIC INLINE VOID LOS_SpinInit(struct Spinlock *lock)
{
    (VOID)lock;

#if LOSCFG_KERNEL_SMP
    LOS_AtomicSet(&(lock->locked), LOS_SPINLOCK_FREE);
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

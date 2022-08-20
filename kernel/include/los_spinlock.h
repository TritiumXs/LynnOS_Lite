/**
 * Copyright (c) 2022 Hunan OpenValley Digital Industry Development Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#if LOSCFG_KERNEL_SMP
    while (LOS_AtomicCmpXchg32bits(&(lock->locked), LOS_SPINLOCK_LOCK, LOS_SPINLOCK_FREE) != TRUE) {
    }
#endif
}

STATIC INLINE INT32 LOS_SpinTrylock(struct Spinlock *lock)
{
    INT32 ret = LOS_OK;

#if LOSCFG_KERNEL_SMP
    ret = (LOS_AtomicCmpXchg32bits(&(lock->locked), LOS_SPINLOCK_LOCK, LOS_SPINLOCK_FREE) != TRUE) ? LOS_OK : LOS_NOK;
#endif

    return ret;
}

STATIC INLINE VOID LOS_SpinUnlock(struct Spinlock *lock)
{
#if LOSCFG_KERNEL_SMP
    LOS_AtomicSet(&(lock->locked), LOS_SPINLOCK_FREE);
#endif
}

STATIC INLINE VOID LOS_SpinLockSave(struct Spinlock *lock, UINT32 *intSave)
{
    *intSave = LOS_IntLock();

#if LOSCFG_KERNEL_SMP
    LOS_SpinLock(lock);
#endif
}

STATIC INLINE VOID LOS_SpinUnlockRestore(struct Spinlock *lock, UINT32 intSave)
{
#if LOSCFG_KERNEL_SMP
    LOS_SpinUnlock(lock);
#endif

    LOS_IntRestore(intSave);
}

STATIC INLINE VOID LOS_SpinInit(struct Spinlock *lock)
{
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

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
#include "los_debug.h"

#include "los_compiler.h"
#include "los_interrupt.h"

#include "los_spinlock_type.h"

#if (LOSCFG_KERNEL_SMP == 1)
#include "los_arch_spinlock.h"
#endif

#include <stdlib.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#if (LOSCFG_SOC_COMPANY_ESPRESSIF == 0)
#ifdef CONFIG_SPIRAM_WORKAROUND_NEED_VOLATILE_SPINLOCK
#define NEED_VOLATILE_MUX volatile
#else
#define NEED_VOLATILE_MUX
#endif /* CONFIG_SPIRAM_WORKAROUND_NEED_VOLATILE_SPINLOCK */

#define SPINLOCK_FREE          0xB33FFFFF
#define SPINLOCK_WAIT_FOREVER  (-1)
#define SPINLOCK_NO_WAIT        0
#define SPINLOCK_INITIALIZER   {.owner = SPINLOCK_FREE,.count = 0}
#define CORE_ID_REGVAL_XOR_SWAP (0xCDCD ^ 0xABAB)

typedef struct {
    NEED_VOLATILE_MUX uint32_t owner;
    NEED_VOLATILE_MUX uint32_t count;
}spinlock_t;
#endif /* LOSCFG_SOC_COMPANY_ESPRESSIF */

/**
 * @brief Gets the spinlock, which blocks the current core until the spinlock is successfully acquired
 * 
 * @param lock Pointer to the spinlock control block
 */
STATIC INLINE VOID LOS_SpinLock(struct Spinlock *lock)
{
    UINT32 intSave;

    intSave = LOS_IntLock();

#if (LOSCFG_KERNEL_SMP == 1)
    while(ArchSpinLock(&(lock->locked)) != TRUE);
#endif

    LOS_IntRestore(intSave);
}

/**
 * @brief Release the spinlock
 * 
 * @param lock Pointer to the spinlock control block
 */
STATIC INLINE VOID LOS_SpinUnlock(struct Spinlock *lock)
{
    UINT32 intSave;
    
    intSave = LOS_IntLock();

#if (LOSCFG_KERNEL_SMP == 1)
    ArchSpinUnlock(&(lock->locked));
#endif

    LOS_IntRestore(intSave);
}

/**
 * @brief Close the interrupt of the current core before holding the spinlock
 * 
 * @param lock Pointer to the spinlock control block
 * @param intSave Save the current interrupt level for recovery
 */
STATIC INLINE VOID LOS_SpinLockSave(struct Spinlock *lock, UINT32 *intSave)
{
    *intSave = LOS_IntLock();

#if (LOSCFG_KERNEL_SMP == 1)
    LOS_SpinLock(lock);
#endif
}

/**
 * @brief 
 * 
 * @param lock Pointer to the spinlock control block
 * @param intSave Restored interrupt level
 */
STATIC INLINE VOID LOS_SpinUnlockRestore(struct Spinlock *lock, UINT32 intSave)
{
#if (LOSCFG_KERNEL_SMP == 1)
    LOS_SpinUnlock(lock);
#endif

    LOS_IntRestore(intSave);
}

/**
 * @brief Initializes the spin lock and assigns a value to the variable in the spin lock control block
 * 
 * @param lock Pointer to the spinlock control block
 */
STATIC INLINE VOID LOS_SpinInit(struct Spinlock *lock)
{
#if (LOSCFG_KERNEL_SMP == 1)
    lock->locked = LOS_SPINLOCK_FREE;
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

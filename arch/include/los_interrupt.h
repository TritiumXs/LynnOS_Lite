/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _LOS_INTERRUPT_H
#define _LOS_INTERRUPT_H
#include "los_config.h"
#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef UINT32 HwiHandle;

typedef UINT16 HwiPrio;

typedef UINT16 HwiMode;

typedef UINT32 HwiArg;

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
typedef VOID (*HwiProcFunc)(VOID *parm);
#else
typedef VOID (*HwiProcFunc)(void);
#endif
typedef struct tagIrqParam {
    HwiArg arg;
    VOID *pDevId;
} HwiIrqParam;

typedef struct {
    UINT32 (*triggerIrq)(HwiHandle hwiNum);
    UINT32 (*clearIrq)(HwiHandle hwiNum);
    UINT32 (*enableIrq)(HwiHandle hwiNum);
    UINT32 (*disableIrq)(HwiHandle hwiNum);
    UINT32 (*setIrqPriority)(HwiHandle hwiNum, UINT8 priority);
    UINT32 (*getCurIrqNum)(VOID);
} HwiControllerOps;

extern HwiControllerOps g_archHwiOps;

/* stack protector */
extern UINT32 __stack_chk_guard;

extern VOID __stack_chk_fail(VOID);

UINT32 ArchIsIntActive(VOID);
#define OS_INT_ACTIVE    (ArchIsIntActive())
#define OS_INT_INACTIVE  (!(OS_INT_ACTIVE))
#define LOS_HwiCreate ArchHwiCreate
#define LOS_HwiDelete ArchHwiDelete
#define LOS_HwiTrigger ArchIntTrigger
#define LOS_HwiEnable ArchIntEnable
#define LOS_HwiDisable ArchIntDisable
#define LOS_HwiClear ArchIntClear
#define LOS_HwiSetPriority ArchIntSetPriority

UINT32 ArchIntLock(VOID);
#define LOS_IntLock ArchIntLock

VOID ArchIntRestore(UINT32 intSave);
#define LOS_IntRestore ArchIntRestore

UINT32 ArchIntUnLock(VOID);
#define LOS_IntUnLock ArchIntUnLock

#define LOS_HwiOpsGet ArchIntOpsGet

/**
 * @ingroup  los_interrupt
 * @brief Delete hardware interrupt.
 *
 * @par Description:
 * This API is used to delete hardware interrupt.
 *
 * @attention
 * <ul>
 * <li>The hardware interrupt module is usable only when the configuration item for hardware interrupt tailoring is enabled.</li>
 * <li>Hardware interrupt number value range: [OS_USER_HWI_MIN,OS_USER_HWI_MAX]. </li>
 * <li>OS_HWI_MAX_NUM specifies the maximum number of interrupts that can be created.</li>
 * <li>Before executing an interrupt on a platform, refer to the chip manual of the platform.</li>
 * </ul>
 *
 * @param  hwiNum   [IN] Type#HwiHandle: hardware interrupt number.
 *
 * @retval #OS_ERRNO_HWI_NUM_INVALID              0x02000900: Invalid interrupt number.
 * @retval #LOS_OK                                0         : The interrupt is successfully delete.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
UINT32 ArchHwiDelete(HwiHandle hwiNum);

/**
 * @ingroup  los_interrupt
 * @brief Create a hardware interrupt.
 *
 * @par Description:
 * This API is used to configure a hardware interrupt and register a hardware interrupt handling function.
 *
 * @attention
 * <ul>
 * <li>The hardware interrupt module is usable only when the configuration item for hardware interrupt tailoring is enabled.</li>
 * <li>Hardware interrupt number value range: [OS_USER_HWI_MIN,OS_USER_HWI_MAX]. </li>
 * <li>OS_HWI_MAX_NUM specifies the maximum number of interrupts that can be created.</li>
 * <li>Before executing an interrupt on a platform, refer to the chip manual of the platform.</li>
 * </ul>
 *
 * @param  hwiNum   [IN] Type#HwiHandle: hardware interrupt number.
 * @param  hwiPrio  [IN] Type#HwiPrio: hardware interrupt priority. Ignore this parameter temporarily.
 * @param  mode     [IN] Type#HwiMode: hardware interrupt mode. Ignore this parameter temporarily.
 * @param  handler  [IN] Type#HwiProcFunc: interrupt handler used when a hardware interrupt is triggered.
 * @param  arg      [IN] Type#HwiArg: input parameter of the interrupt handler used when a hardware interrupt is triggered.
 *
 * @retval #OS_ERRNO_HWI_PROC_FUNC_NULL               0x02000901: Null hardware interrupt handling function.
 * @retval #OS_ERRNO_HWI_NUM_INVALID                  0x02000900: Invalid interrupt number.
 * @retval #OS_ERRNO_HWI_NO_MEMORY                    0x02000903: Insufficient memory for hardware interrupt creation.
 * @retval #OS_ERRNO_HWI_ALREADY_CREATED              0x02000904: The interrupt handler being created has already been created.
 * @retval #LOS_OK                                    0         : The interrupt is successfully created.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
UINT32 ArchHwiCreate(HwiHandle hwiNum,
                     HwiPrio hwiPrio,
                     HwiMode hwiMode,
                     HwiProcFunc hwiHandler,
                     HwiIrqParam *irqParam);

STATIC INLINE UINT32 ArchIntTrigger(HwiHandle hwiNum)
{
    if (g_archHwiOps.triggerIrq == NULL) {
        return LOS_NOK;
    }
    return g_archHwiOps.triggerIrq(hwiNum);
}

STATIC INLINE UINT32 ArchIntEnable(HwiHandle hwiNum)
{
    if (g_archHwiOps.enableIrq == NULL) {
        return LOS_NOK;
    }
    return g_archHwiOps.enableIrq(hwiNum);
}

STATIC INLINE UINT32 ArchIntDisable(HwiHandle hwiNum)
{
    if (g_archHwiOps.disableIrq == NULL) {
        return LOS_NOK;
    }
    return g_archHwiOps.disableIrq(hwiNum);
}

STATIC INLINE UINT32 ArchIntClear(HwiHandle hwiNum)
{
    if (g_archHwiOps.clearIrq == NULL) {
        return LOS_NOK;
    }
    return g_archHwiOps.clearIrq(hwiNum);
}

STATIC INLINE UINT32 ArchIntSetPriority(HwiHandle hwiNum, HwiPrio priority)
{
    if (g_archHwiOps.setIrqPriority == NULL) {
        return LOS_NOK;
    }
    return g_archHwiOps.setIrqPriority(hwiNum, priority);
}

STATIC INLINE HwiControllerOps *ArchIntOpsGet(VOID)
{
    return &g_archHwiOps;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_INTERRUPT_H */

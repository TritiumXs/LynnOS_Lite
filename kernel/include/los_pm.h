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

#ifndef _LOS_PM_H
#define _LOS_PM_H

#include "los_config.h"
#include "los_compiler.h"
#include "los_list.h"
#include "los_error.h"

typedef enum {
    LOS_SYS_NORMAL_SLEEP = 0,
    LOS_SYS_DEEP_SLEEP,
    LOS_SYS_SHUTDOWN,
} LOS_SysSleepEnum;

#define LOS_PM_FLAGS_LOW_POWER_TIMER  0x1
#define LOS_PM_FLAGS_SYSCTRL          0x2

typedef UINT32 (*PmDeviceSuspend)(VOID);

typedef VOID (*PmDeviceResume)(VOID);

typedef struct {
    UINT32          flags;         /* This field must be at the beginning of the structure header,
                                    * It needs to be initialized to 0.
                                    */
    PmDeviceSuspend deepSuspend;
    PmDeviceResume  deepResume;
    PmDeviceSuspend shutdownSuspend;
    PmDeviceResume  shutdownResume;
    LOS_DL_LIST     list;            /*　Device node linked list, do not need to initialize */
} LosPmDeviceOpt;

#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)

typedef VOID (*PmTickTimerLock)(VOID);

typedef VOID (*PmTickTimerUnlock)(VOID);

typedef VOID (*PmLowFreqTimerStart)(VOID *arg, UINT64 sleepTime);

typedef VOID (*PmLowFreqTimerStop)(VOID *arg);

typedef UINT64 (*PmLowFreqTimerCycleGet)(VOID);

typedef struct {
    UINT32                 flags;           /* This field must be at the beginning of the structure header,
                                             * It needs to be initialized to LOS_PM_FLAGS_LOW_POWER_TIMER.
                                             */
    PmLowFreqTimerStart    start;           /* Start the low power timer */
    PmLowFreqTimerStop     stop;            /* Stop the low power timer */
    PmLowFreqTimerCycleGet getTimeCycle;    /* Gets the running time of the low power timer in unit cycle */
    UINT32                 freq;            /* The frequency of the low power timer */
    VOID                   *arg;            /* The parameter of the low power timer */
    PmTickTimerLock        lock;            /* Pause the system tick timer */
    PmTickTimerUnlock      unlock;          /* Restore the system tick timer */
} LosPmLowFreqTimer;
#endif

typedef VOID (*PmSysctrlSuspend)(LOS_SysSleepEnum mode);

typedef VOID (*PmSysctrlResume)(LOS_SysSleepEnum mode);
typedef struct {
    UINT32           flags;    /* This field must be at the beginning of the structure header,
                                * It needs to be initialized to LOS_PM_FLAGS_SYSCTRL.
                                */
    PmSysctrlSuspend suspend;
    PmSysctrlResume  resume;
} LosPmSysctrl;

VOID OsPmRestore(VOID);

VOID OsPmEnter(VOID);

/**
 * @ingroup los_pm
 * Pm error code: Invalid low power mode.
 *
 * Value: 0x02002001
 *
 * Solution: This error code is not in use temporarily.
 */
#define LOS_ERRNO_PM_INVALID_MODE       LOS_ERRNO_OS_ERROR(LOS_MOD_PM, 0x01)

/**
 * @ingroup los_pm
 * Pm error code: Invalid input parameter.
 *
 * Value: 0x02002002
 *
 * Solution: This error code is not in use temporarily.
 */
#define LOS_ERRNO_PM_INVALID_PARAM      LOS_ERRNO_OS_ERROR(LOS_MOD_PM, 0x02)

/**
 * @ingroup los_pm
 * Pm error code: The current mode is unlocked.
 *
 * Value: 0x02002003
 *
 * Solution: This error code is not in use temporarily.
 */
#define LOS_ERRNO_PM_NOT_LOCK           LOS_ERRNO_OS_ERROR(LOS_MOD_PM, 0x03)

/**
 * @ingroup los_pm
 * Pm error code: The lock limit has been exceeded.
 *
 * Value: 0x02002004
 *
 * Solution: This error code is not in use temporarily.
 */
#define LOS_ERRNO_PM_LOCK_LIMIT         LOS_ERRNO_OS_ERROR(LOS_MOD_PM, 0x04)

/**
 * @ingroup los_pm
 * Pm error code: Invalid device node.
 *
 * Value: 0x02002005
 *
 * Solution: This error code is not in use temporarily.
 */
#define LOS_ERRNO_PM_INVALID_NODE       LOS_ERRNO_OS_ERROR(LOS_MOD_PM, 0x05)

/**
 * @ingroup los_pm
 * Pm error code: Invalid flags.
 *
 * Value: 0x02002006
 *
 * Solution: This error code is not in use temporarily.
 */
#define LOS_ERRNO_PM_INVALID_FLAG       LOS_ERRNO_OS_ERROR(LOS_MOD_PM, 0x06)

/**
 * @ingroup los_pm
 * Pm error code: This mode has already been enabled.
 *
 * Value: 0x02002007
 *
 * Solution: This error code is not in use temporarily.
 */
#define LOS_ERRNO_PM_MODE_ALREADY_ENABLE LOS_ERRNO_OS_ERROR(LOS_MOD_PM, 0x07)

/**
 * @ingroup los_pm
 * Pm error code: This mode is not enabled.
 *
 * Value: 0x02002008
 *
 * Solution: This error code is not in use temporarily.
 */
#define LOS_ERRNO_PM_MODE_NOT_ENABLE     LOS_ERRNO_OS_ERROR(LOS_MOD_PM, 0x08)

/**
 * @ingroup los_pm
 * @brief System power management framework initializes.
 *
 * @par Description:
 * This API is used to initialize the system power management framework.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval error code, LOS_OK means success.
 * @par Dependency:
 * <ul><li>los_pm.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
UINT32 LOS_PmInit(VOID);

/**
 * @ingroup los_pm
 * @brief Register a power management node.
 *
 * @par Description:
 * This API is used to register a power management node.
 *
 * @attention None.
 *
 * @param  opt [IN] power management node.
 *
 * @retval error code, LOS_OK means success.
 * @par Dependency:
 * <ul><li>los_pm.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_PmUnregistered
 */
UINT32 LOS_PmRegistered(LosPmDeviceOpt *opt);

/**
 * @ingroup los_pm
 * @brief Unregister a power management node.
 *
 * @par Description:
 * This API is used to unregister a power management node.
 *
 * @attention None.
 *
 * @param  opt [IN] power management node.
 *
 * @retval error code, LOS_OK means success.
 * @par Dependency:
 * <ul><li>los_pm.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_PmRegistered
 */
UINT32 LOS_PmUnregistered(LosPmDeviceOpt *opt);

/**
 * @ingroup los_pm
 * @brief Gets the power consumption mode of the current system.
 *
 * @par Description:
 * This API is used to get the power consumption mode of the current system.
 *
 * @attention None.
 *
 * @param None.
 *
 * @retval error code, LOS_OK means success.
 * @par Dependency:
 * <ul><li>los_pm.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_PmModeSet
 */
LOS_SysSleepEnum LOS_PmModeGet(VOID);

/**
 * @ingroup los_pm
 * @brief Setting power mode.
 *
 * @par Description:
 * This API is used to set power mode.
 *
 * @attention None.
 *
 * @param  mode [IN] low power mode.
 *
 * @retval error code, LOS_OK means success.
 * @par Dependency:
 * <ul><li>los_pm.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_PmModeGet
 */
UINT32 LOS_PmModeSet(LOS_SysSleepEnum mode);

/**
 * @ingroup los_pm
 * @brief Request to obtain the lock in this mode, so that the system will not enter
 * this mode when it enters the idle task next time.
 *
 * @par Description:
 * This API is used to obtain the lock in this mode.
 *
 * @attention None.
 *
 * @param  name [IN] Who request the lock.
 *
 * @retval error code, LOS_OK means success.
 * @par Dependency:
 * <ul><li>los_pm.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_PmLockRelease
 */
UINT32 LOS_PmLockRequest(const CHAR *name);

/**
 * @ingroup los_pm
 * @brief Release the lock in this mode so that the next time the system enters the idle task, it will enter this mode.
 *
 * @par Description:
 * This API is used to release the lock in this mode.
 *
 * @attention None.
 *
 * @param  name [IN] Who release the lock.
 *
 * @retval error code, LOS_OK means success.
 * @par Dependency:
 * <ul><li>los_pm.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_PmLockRequest
 */
UINT32 LOS_PmLockRelease(const CHAR *name);

#if (LOSCFG_KERNEL_PM_DEBUG == 1)
/**
 * @ingroup los_pm
 * @brief Output the locking information of the pm lock.
 *
 * @par Description:
 * This API is used to output the locking information of the pm lock.
 *
 * @attention None.
 *
 * @param  name [IN] Who release the lock.
 *
 * @retval error code, LOS_OK means success.
 * @par Dependency:
 * <ul><li>los_pm.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_PmLockRequest
 */
VOID LOS_PmLockInfoShow(VOID);
#endif
#endif

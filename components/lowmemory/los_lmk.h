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

#ifndef _LOS_LMK_H
#define _LOS_LMK_H

#include "los_config.h"
#include "los_compiler.h"
#include "los_list.h"

typedef struct {
    /*
     * The priority in the LMK list, the higher priority with a smaller number.
     */
    UINT32 priority;
    /*
     * Release the memory of tasks in the LMK list. Return LOS_OK for a successful release.
     */
    UINT32 (*freeMemByKillingTask)(VOID);
    /*
     * Restore the tasks killed by the high memory task where the high memory task quits.
     */
    VOID (*restoreKilledTask)(VOID);
    /*
     * LosLmkOpsNode node.
     */
    LOS_DL_LIST node;
} LosLmkOpsNode;

/**
 * @ingroup los_lmk
 * @brief Register a low memory killer node.
 *
 * @par Description:
 * This API is used to register a low memory killer node. A LosLmkOpsNode node
 * can be registered only once.
 *
 * @attention None.
 *
 * @param  lmkNode [IN] The LosLmkOpsNode node to be registered.
 *
 * @retval LOS_NOK Failed to register the LosLmkOpsNode node.
 * @retval LOS_OK The lLosLmkOpsNode node is registered successfully.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
UINT32 LOS_LmkOpsNodeRegister(LosLmkOpsNode *lmkNode);

/**
 * @ingroup los_lmk
 * @brief Unregister a low memory killer node.
 *
 * @par Description:
 * This API is used to unregister a low memory killer node.
 *
 * @attention None.
 *
 * @param  lmkNode [IN] The LosLmkOpsNode node to be registered.
 *
 * @retval LOS_NOK Failed to unregister the LosLmkOpsNode node.
 * @retval LOS_OK The LosLmkOpsNode node is unregistered successfully.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
UINT32 LOS_LmkOpsNodeUnregister(LosLmkOpsNode *lmkNode);

/**
 * @ingroup los_lmk
 * @brief Initialize low memory killer framework.
 *
 * @par Description:
 * This API is used to initialize the low memory killer framework.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
VOID OsLmkInit(VOID);

/**
 * @ingroup los_lmk
 * @brief Get the LosLmkOpsNode list.
 *
 * @par Description:
 * This API is used to get the LosLmkOpsNode list.
 *
 * @attention None.
 *
 * @param  list.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
VOID OsLmkOpsListGet(LOS_DL_LIST *list);

/**
 * @ingroup los_lmk
 * @brief Restore the tasks killed by the task which triggers low memory killer.
 *
 * @par Description:
 * This API is used to restore the tasks killed by the task which triggers low memory killer.
 * This function will be invoked by the developer as needed.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
VOID OsLmkTasksRestore(VOID);

/**
 * @ingroup los_lmk
 * @brief Kill the tasks to release the used memory.
 *
 * @par Description:
 *  This API is used to kill the tasks to release the used memory when low memory killer is triggered. 
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval LOS_OK  Release memory successfully.
 * @retval LOS_NOK No memory is released.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
UINT32 OsLmkTasksKill(VOID);
#endif

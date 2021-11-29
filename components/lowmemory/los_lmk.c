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

#include "los_lmk.h"

#if (LOSCFG_KERNEL_LMK == 1)
static LOS_DL_LIST g_losLmkOps;

BOOL OsIsLmkOpsNodeRegistered(LosLmkOpsNode *lmkNode)
{
    if (lmkNode == NULL) {
        return FALSE;
    }

    if (LOS_ListEmpty(&g_losLmkOps)) {
        return FALSE;
    }

    for (LOS_DL_LIST *listNode = &g_losLmkOps; listNode != g_losLmkOps.pstPrev; listNode = listNode->pstNext) {
        LosLmkOpsNode *opsNode = LOS_DL_LIST_ENTRY(listNode->pstNext, LosLmkOpsNode, node);
        if (lmkNode == opsNode) {
            return TRUE;
        }
    }
    return FALSE;
}

UINT32 LOS_LmkOpsNodeRegister(LosLmkOpsNode *lmkNode)
{
    if (lmkNode == NULL) {
        return LOS_NOK;
    }

    if (OsIsLmkOpsNodeRegistered(lmkNode)) {
        return LOS_NOK;
    }

    if (LOS_ListEmpty(&g_losLmkOps)) {
        LOS_ListHeadInsert(&g_losLmkOps, &lmkNode->node);
        return LOS_OK;
    }

    LosLmkOpsNode *opsNode = LOS_DL_LIST_ENTRY(LOS_DL_LIST_FIRST(&g_losLmkOps), LosLmkOpsNode, node);
    if (lmkNode->priority <= opsNode->priority) {
        LOS_ListHeadInsert(&g_losLmkOps, &lmkNode->node);
        return LOS_OK;
    }

    for (LOS_DL_LIST *listNode = &g_losLmkOps; listNode != g_losLmkOps.pstPrev; listNode = listNode->pstNext) {
        opsNode = LOS_DL_LIST_ENTRY(listNode->pstNext, LosLmkOpsNode, node);
        if (lmkNode->priority < opsNode->priority) {
            LOS_ListHeadInsert(listNode, &lmkNode->node);
            break;
        }
    }
    return LOS_OK;
}

UINT32 LOS_LmkOpsNodeUnregister(LosLmkOpsNode *lmkNode)
{
    if (lmkNode == NULL) {
        return LOS_NOK;
    }

    if (LOS_ListEmpty(&g_losLmkOps) || !OsIsLmkOpsNodeRegistered(lmkNode)) {
        return LOS_NOK;
    }

    LOS_ListDelete(&lmkNode->node);
    return LOS_OK;
}

UINT32 OsLmkTasksKill(VOID)
{
    LosLmkOpsNode *opsNode = NULL;
    UINT32 ret = LOS_NOK;

    LOS_DL_LIST_FOR_EACH_ENTRY(opsNode, &g_losLmkOps, LosLmkOpsNode, node) {
        if (opsNode->freeMemByKillingTask != NULL) {
            ret &= opsNode->freeMemByKillingTask();
        }
    }
    return ret;
}

VOID OsLmkTasksRestore(VOID)
{
    LosLmkOpsNode *opsNode = NULL;

    LOS_DL_LIST_FOR_EACH_ENTRY(opsNode, &g_losLmkOps, LosLmkOpsNode, node) {
        if (opsNode->restoreKilledTask != NULL) {
            opsNode->restoreKilledTask();
        }
    }
}

VOID OsLmkInit(VOID)
{
    LOS_ListInit(&g_losLmkOps);
}

VOID OsLmkOpsListGet(LOS_DL_LIST *list)
{
    list = &g_losLmkOps;
}
#endif

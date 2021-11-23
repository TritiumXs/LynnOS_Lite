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
#include "securec.h"
#include "los_memory.h"
#include "los_task.h"

#if (LOSCFG_KERNEL_LMK == 1)

LosLmkOps g_losLmkOps;
TSK_INIT_PARAM_S *g_killedTaskInitParams = NULL;

STATIC VOID OsBackupTaskInitParam(TSK_INIT_PARAM_S *tskInitParams, LosTaskCB *taskCB)
{
    tskInitParams->pfnTaskEntry = taskCB->taskEntry;
    tskInitParams->uwStackSize = taskCB->stackSize;
    tskInitParams->pcName = taskCB->taskName;
    tskInitParams->usTaskPrio = taskCB->priority;
    tskInitParams->uwResved = LOS_TASK_ATTR_KILLABLE;
    tskInitParams++;
}

STATIC VOID *OsMemMallocTaskInitParam(VOID)
{
    UINT32 size;
    UINT32 killableTaskNum;
    LOS_DL_LIST *listNodeHead = NULL;
    LOS_DL_LIST *listNode = NULL;
    TSK_INIT_PARAM_S *tskInitParams = NULL;

    listNodeHead = &g_losLmkOps.killableTaskList;
    LOS_DL_LIST_FOR_EACH(listNode, listNodeHead) {
        killableTaskNum++;
    }
    if (killableTaskNum <= 0) {
        return NULL;
    }
    size = sizeof(TSK_INIT_PARAM_S) * killableTaskNum;
    tskInitParams = (TSK_INIT_PARAM_S *)LOS_MemAlloc(m_aucSysMem0, size);

    if (tskInitParams == NULL) {
        return NULL;
    }
    g_killedTaskInitParams = tskInitParams;
    (VOID)memset_s((VOID *)tskInitParams, size, 0, size);
    return (VOID *)tskInitParams;
}

STATIC VOID OsKilledTasksRestore(VOID)
{
    TSK_INIT_PARAM_S *tskInitParams = NULL;
    UINT32 taskId;
    UINT32 intSave;

    tskInitParams = g_killedTaskInitParams;

    intSave = LOS_IntLock();
    while (tskInitParams != NULL) {
        (VOID) LOS_TaskCreate(&taskId, tskInitParams);
        tskInitParams++;
    }
    LOS_IntRestore(intSave);

    LOS_MemFree(m_aucSysMem0, g_killedTaskInitParams);
    g_killedTaskInitParams = NULL;
}

STATIC UINT32 OsKillableTasksKill(VOID)
{
    UINT32 ret;
    UINT32 intSave;
    LOS_DL_LIST *listNodeHead = NULL;
    LosTaskCB *taskCB = NULL;
    TSK_INIT_PARAM_S *tskInitParams = NULL;

    ret = LOS_OK;
    listNodeHead = &g_losLmkOps.killableTaskList;
    tskInitParams = (TSK_INIT_PARAM_S *)OsMemMallocTaskInitParam();

    while (!LOS_ListEmpty(listNodeHead)) {
        intSave = LOS_IntLock();
        taskCB = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(listNodeHead));
        LOS_ListDelete(LOS_DL_LIST_FIRST(listNodeHead));
        LOS_IntRestore(intSave);

        ret = LOS_MemFreeByTaskID(m_aucSysMem0, taskCB->taskID);
        if (ret != LOS_OK) {
            break;
        }

        OsBackupTaskInitParam(tskInitParams, taskCB);
        ret = LOS_TaskDelete(taskCB->taskID);
        if (ret != LOS_OK) {
            break;
        }
    }

    if (ret != LOS_OK) {
        OsKilledTasksRestore();
    } else {
        g_losLmkOps.highMemTaskId = LOS_CurTaskIDGet();
    }

    return ret;
}

UINT32 OsLmkInit(VOID)
{
    UINT32 ret = LOS_OK;

    (VOID)memset_s(&g_losLmkOps, sizeof(LosLmkOps), 0, sizeof(LosLmkOps));
    g_losLmkOps.release = OsKillableTasksKill;
    g_losLmkOps.restore = OsKilledTasksRestore;

    LOS_ListInit(&g_losLmkOps.killableTaskList);

    return ret;
}
#endif

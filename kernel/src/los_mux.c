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

#include "los_mux.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_interrupt.h"
#include "los_memory.h"
#include "los_sched.h"
#include "los_spinlock.h"

#if (LOSCFG_BASE_IPC_MUX == 1)

LITE_OS_SEC_BSS       LosMuxCB*   g_allMux = NULL;
LITE_OS_SEC_DATA_INIT LOS_DL_LIST g_unusedMuxList;

STATIC VOID MuxLock(LosMuxCB *mux, UINT32 *intSave)
{
    LOS_SpinLockSave(&(mux->lock), intSave);
}

STATIC VOID MuxUnlock(LosMuxCB *mux, UINT32 intSave)
{
    LOS_SpinUnlockRestore(&(mux->lock), intSave);
}

/*****************************************************************************
 Function      : OsMuxInit
 Description  : Initializes the mutex
 Input        : None
 Output       : None
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsMuxInit(VOID)
{
    LosMuxCB *muxNode = NULL;
    UINT32 index;

    LOS_ListInit(&g_unusedMuxList);

    if (LOSCFG_BASE_IPC_MUX_LIMIT == 0) {
        return LOS_ERRNO_MUX_MAXNUM_ZERO;
    }

    g_allMux = (LosMuxCB *)LOS_MemAlloc(m_aucSysMem0, (LOSCFG_BASE_IPC_MUX_LIMIT * sizeof(LosMuxCB)));
    if (g_allMux == NULL) {
        return LOS_ERRNO_MUX_NO_MEMORY;
    }

    for (index = 0; index < LOSCFG_BASE_IPC_MUX_LIMIT; index++) {
        muxNode = ((LosMuxCB *)g_allMux) + index;
        muxNode->muxID = index;
        muxNode->owner = (LosTaskCB *)NULL;
        muxNode->muxStat = OS_MUX_UNUSED;
#if (LOSCFG_MUTEX_CREATE_TRACE == 1)
        muxNode->createInfo = 0;
#endif
        LOS_ListTailInsert(&g_unusedMuxList, &muxNode->muxList);
    }
    return LOS_OK;
}

/*****************************************************************************
 Function     : LOS_MuxCreate
 Description  : Create a mutex
 Input        : None
 Output       : muxHandle ------ Mutex operation handle
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_MuxCreate(UINT32 *muxHandle)
{
    UINT32 intSave;
    LosMuxCB *muxCreated = NULL;
    LOS_DL_LIST *unusedMux = NULL;
    UINT32 errNo;
    UINT32 errLine;

    if (muxHandle == NULL) {
        return LOS_ERRNO_MUX_PTR_NULL;
    }

    OsSchedLock(&intSave);
    if (LOS_ListEmpty(&g_unusedMuxList)) {
        OsSchedUnlock(intSave);
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_MUX_ALL_BUSY);
    }

    // 取出第一个空闲的控制块
    unusedMux = LOS_DL_LIST_FIRST(&(g_unusedMuxList));
    LOS_ListDelete(unusedMux);
    muxCreated = (GET_MUX_LIST(unusedMux));

    // 初始化控制块
    muxCreated->muxCount = 0;
    muxCreated->muxStat = OS_MUX_USED;
    muxCreated->priority = 0;
    muxCreated->owner = (LosTaskCB *)NULL;
    LOS_ListInit(&muxCreated->muxList);
    LOS_SpinInit(&(muxCreated->lock));// 初始化锁

    // 保存互斥锁句柄
    *muxHandle = (UINT32)muxCreated->muxID;

    OsSchedUnlock(intSave);

    OsHookCall(LOS_HOOK_TYPE_MUX_CREATE, muxCreated);
    return LOS_OK;

ERR_HANDLER:
    OS_RETURN_ERROR_P2(errLine, errNo);
}

/*****************************************************************************
 Function     : LOS_MuxDelete
 Description  : Delete a mutex
 Input        : muxHandle ------Mutex operation handle
 Output       : None
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_MuxDelete(UINT32 muxHandle)
{
    UINT32 intSave;
    LosMuxCB *muxDeleted = NULL;
    UINT32 errNo;
    UINT32 errLine;

    if (muxHandle >= (UINT32)LOSCFG_BASE_IPC_MUX_LIMIT) {
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_MUX_INVALID);
    }

    muxDeleted = GET_MUX(muxHandle);

    // 持有自旋锁
    MuxLock(muxDeleted, &intSave);

    if (muxDeleted->muxStat == OS_MUX_UNUSED) {
        MuxUnlock(muxDeleted, intSave);
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_MUX_INVALID);
    }

    if ((!LOS_ListEmpty(&muxDeleted->muxList)) || muxDeleted->muxCount) {
        MuxUnlock(muxDeleted, intSave);
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_MUX_PENDED);
    }

    muxDeleted->muxStat = OS_MUX_UNUSED;
    MuxUnlock(muxDeleted, intSave);

    // 回收控制块
    OsSchedLock(&intSave);
    LOS_ListAdd(&g_unusedMuxList, &muxDeleted->muxList);
    OsSchedUnlock(intSave);

    OsHookCall(LOS_HOOK_TYPE_MUX_DELETE, muxDeleted);
    return LOS_OK;

ERR_HANDLER:
    OS_RETURN_ERROR_P2(errLine, errNo);
}

STATIC_INLINE UINT32 OsMuxValidCheck(LosMuxCB *muxPended)
{
    LosTask *losTask = OsTaskGet();

    if (muxPended->muxStat == OS_MUX_UNUSED) {
        return LOS_ERRNO_MUX_INVALID;
    }

    if (OS_INT_ACTIVE) {
        return LOS_ERRNO_MUX_IN_INTERR;
    }

    if (OsTaskIslock(ArchCurrCpuid())) {
        PRINT_ERR("!!!LOS_ERRNO_MUX_PEND_IN_LOCK!!!\n");
        return LOS_ERRNO_MUX_PEND_IN_LOCK;
    }

    if (OsTaskIsSystemTask(losTask->runTask)) {
        return LOS_ERRNO_MUX_PEND_IN_SYSTEM_TASK;
    }

    return LOS_OK;
}

/*****************************************************************************
 Function     : LOS_MuxPend
 Description  : Specify the mutex P operation
 Input        : muxHandle ------ Mutex operation handleone
              : timeOut   ------- waiting time
 Output       : None
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_MuxPend(UINT32 muxHandle, UINT32 timeout)
{
    UINT32 intSave, intSave1;
    LosMuxCB *muxPended = NULL;
    UINT32 retErr;
    LosTaskCB *runningTask = NULL;
    LosTask *losTask = OsTaskGet();

    if (muxHandle >= (UINT32)LOSCFG_BASE_IPC_MUX_LIMIT) {
        OS_RETURN_ERROR(LOS_ERRNO_MUX_INVALID);
    }

    muxPended = GET_MUX(muxHandle);

    MuxLock(muxPended, &intSave);

    retErr = OsMuxValidCheck(muxPended);
    if (retErr) {
        goto ERROR_MUX_PEND;
    }

    // 获取当前任务句柄
    runningTask = (LosTaskCB *)losTask->runTask;

    // 当前没有任务持锁
    if (muxPended->muxCount == 0)
    {
        muxPended->muxCount++;
        muxPended->owner = runningTask;
        muxPended->priority = runningTask->priority;

        MuxUnlock(muxPended, intSave);

        goto HOOK;
    }

    // 当前任务已经持锁
    if (muxPended->owner == runningTask)
    {
        muxPended->muxCount++;

        MuxUnlock(muxPended, intSave);
        
        goto HOOK;
    }

    if (!timeout)
    {
        retErr = LOS_ERRNO_MUX_UNAVAILABLE;
        goto ERROR_MUX_PEND;
    }

    runningTask->taskMux = (VOID *)muxPended;

    /* 调度相关 */
    OsSchedLock(&intSave1);

    // 提高当前持锁的任务的优先级
    if (muxPended->owner->priority > runningTask->priority) {
        (VOID)OsSchedModifyTaskSchedParam(muxPended->owner, runningTask->priority);
    }

    // 挂起任务
    OsSchedTaskWait(&muxPended->muxList, timeout);

    OsSchedUnlock(intSave1);
    /* 调度相关 */

    MuxUnlock(muxPended, intSave);

    OsHookCall(LOS_HOOK_TYPE_MUX_PEND, muxPended, timeout);

    // 当前核心触发一次调度
    LOS_Schedule();

    // 执行到此处说明当前任务持锁操作已经完成(或者超时了)
    OsSchedLock(&intSave1);

    // 判断当前任务是否处于超时状态
    if (OsSchedTimeoutHandle(runningTask) == OS_TASK_STATUS_TIMEOUT)
    {
        // 超时了
        retErr = LOS_ERRNO_MUX_TIMEOUT;
        OsSchedUnlock(intSave1);
        OS_RETURN_ERROR(retErr);
    }

    OsSchedUnlock(intSave1);

    return LOS_OK;

HOOK:
    OsHookCall(LOS_HOOK_TYPE_MUX_PEND, muxPended, timeout);
    return LOS_OK;

ERROR_MUX_PEND:
    MuxUnlock(muxPended, intSave);
    OS_RETURN_ERROR(retErr);
}

/*****************************************************************************
 Function     : LOS_MuxPost
 Description  : Specify the mutex V operation,
 Input        : muxHandle ------ Mutex operation handle
 Output       : None
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_MuxPost(UINT32 muxHandle)
{
    UINT32 intSave, intSave1;
    LosMuxCB *muxPosted = GET_MUX(muxHandle);
    LosTaskCB *resumedTask = NULL;
    LosTaskCB *runningTask = NULL;

    LosTask *losTask = OsTaskGet();

    MuxLock(muxPosted, &intSave);

    if ((muxHandle >= (UINT32)LOSCFG_BASE_IPC_MUX_LIMIT) ||
        (muxPosted->muxStat == OS_MUX_UNUSED)) {

        MuxUnlock(muxPosted, intSave);
        
        OS_RETURN_ERROR(LOS_ERRNO_MUX_INVALID);
    }

    if (OS_INT_ACTIVE)
    {
        MuxUnlock(muxPosted, intSave);

        OS_RETURN_ERROR(LOS_ERRNO_MUX_IN_INTERR);
    }

    runningTask = (LosTaskCB *)losTask->runTask;
    
    if ((muxPosted->muxCount == 0) || (muxPosted->owner != runningTask))
    {
        MuxUnlock(muxPosted, intSave);

        OS_RETURN_ERROR(LOS_ERRNO_MUX_INVALID);
    }

    if (--(muxPosted->muxCount) != 0)
    {
        MuxUnlock(muxPosted, intSave);

        OsHookCall(LOS_HOOK_TYPE_MUX_POST, muxPosted);
        return LOS_OK;
    }

    /* 调度相关 */
    OsSchedLock(&intSave1);

    // 修改优先级
    if ((muxPosted->owner->priority) != muxPosted->priority) {
        (VOID)OsSchedModifyTaskSchedParam(muxPosted->owner, muxPosted->priority);
    }

    OsSchedUnlock(intSave1);
    /* 调度相关 */

    if (!LOS_ListEmpty(&muxPosted->muxList))
    {
        resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&(muxPosted->muxList)));

        muxPosted->muxCount = 1;
        muxPosted->owner = resumedTask;
        muxPosted->priority = resumedTask->priority;
        resumedTask->taskMux = NULL;

        /* 调度相关 */
        OsSchedLock(&intSave1);
        OsSchedTaskWake(resumedTask);
        OsSchedUnlock(intSave1);
        /* 调度相关 */

        MuxUnlock(muxPosted, intSave);

        OsHookCall(LOS_HOOK_TYPE_MUX_POST, muxPosted);

        // 让被释放的任务所在的核心执行一次调度
        if (OS_SCHED_RQ_IS_RUNNING(resumedTask->schedRunqueueNum))
        {
            LOS_MpSchedule(CPUID_TO_AFFI_MASK(resumedTask->schedRunqueueNum));

            // 当前核心进行任务调度
            if (resumedTask->schedRunqueueNum == ArchCurrCpuid())
            {
                LOS_Schedule();
            }
        }
    } 
    else 
    {
        muxPosted->owner = NULL;

        MuxUnlock(muxPosted, intSave);
    }

    return LOS_OK;
}
#endif /* (LOSCFG_BASE_IPC_MUX == 1) */

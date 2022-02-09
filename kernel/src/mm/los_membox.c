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

#include "los_membox.h"
#include "securec.h"
#include "los_interrupt.h"
#include "los_context.h"
#include "los_debug.h"
#include "los_task.h"


/* The magic length is 32 bits, the lower 8 bits are used to save the owner task ID,
   and the other 24 bits are used to set the magic number for verification. */
#define OS_MEMBOX_MAGIC         0xa55a5a00
#define OS_MEMBOX_TASKID_BITS   8
#define OS_MEMBOX_MAX_TASKID    ((1 << OS_MEMBOX_TASKID_BITS) - 1)
#define OS_MEMBOX_TASKID_GET(addr) (((UINTPTR)(addr)) & OS_MEMBOX_MAX_TASKID)

STATIC INLINE VOID OsMemBoxSetMagic(LosMemBoxNode *node)
{
    UINT8 queueId = (UINT8)LOS_CurTaskIdGet();
    node->pstNext = (LosMemBoxNode *)(OS_MEMBOX_MAGIC | queueId);
}

STATIC INLINE UINT32 OsMemBoxCheckMagic(LosMemBoxNode *node)
{
    UINT32 queueId = OS_MEMBOX_TASKID_GET(node->pstNext);
    if (queueId > (LOSCFG_BASE_CORE_TSK_LIMIT + 1)) {
        return LOS_NOK;
    } else {
        return (node->pstNext == (LosMemBoxNode *)(OS_MEMBOX_MAGIC | queueId)) ? LOS_OK : LOS_NOK;
    }
}

#define OS_MEMBOX_USER_ADDR(addr) \
    ((VOID *)((UINT8 *)(addr) + OS_MEMBOX_NODE_HEAD_SIZE))
#define OS_MEMBOX_NODE_ADDR(addr) \
    ((LosMemBoxNode *)(VOID *)((UINT8 *)(addr) - OS_MEMBOX_NODE_HEAD_SIZE))
#define MEMBOX_LOCK(state)       ((state) = ArchIntLock())
#define MEMBOX_UNLOCK(state)     ArchIntRestore(state)

STATIC INLINE UINT32 OsCheckBoxMem(const LosMemBoxInfo *boxInfo, const VOID *node)
{
    UINT32 offset;

    if (boxInfo->blkSize == 0) {
        return LOS_NOK;
    }

    offset = (UINT32)((UINTPTR)node - (UINTPTR)(boxInfo + 1));
    if ((offset % boxInfo->blkSize) != 0) {
        return LOS_NOK;
    }

    if ((offset / boxInfo->blkSize) >= boxInfo->blkNum) {
        return LOS_NOK;
    }

    return OsMemBoxCheckMagic((LosMemBoxNode *)node);
}

#if (LOSCFG_PLATFORM_EXC == 1)
STATIC LosMemBoxInfo *g_memBoxHead = NULL;
STATIC VOID OsMemBoxAdd(VOID *pool)
{
    LosMemBoxInfo *nextPool = g_memBoxHead;
    LosMemBoxInfo *curPool = NULL;

    while (nextPool != NULL) {
        curPool = nextPool;
        nextPool = nextPool->nextMemBox;
    }

    if (curPool == NULL) {
        g_memBoxHead = pool;
    } else {
        curPool->nextMemBox = pool;
    }

    ((LosMemBoxInfo *)pool)->nextMemBox = NULL;
}
#endif

UINT32 LOS_MemboxInit(VOID *pool, UINT32 poolSize, UINT32 blkSize)
{
    LosMemBoxInfo *boxInfo = (LosMemBoxInfo *)pool;
    LosMemBoxNode *node = NULL;
    UINT32 index;
    UINT32 intSave;

    if (pool == NULL) {
        return LOS_NOK;
    }

    if (blkSize == 0) {
        return LOS_NOK;
    }

    if (poolSize < sizeof(LosMemBoxInfo)) {
        return LOS_NOK;
    }

    MEMBOX_LOCK(intSave);
    boxInfo->blkSize = LOS_MEMBOX_ALIGNED(blkSize + OS_MEMBOX_NODE_HEAD_SIZE);
    boxInfo->blkNum = (poolSize - sizeof(LosMemBoxInfo)) / boxInfo->blkSize;
    boxInfo->blkCnt = 0;
    if (boxInfo->blkNum == 0) {
        MEMBOX_UNLOCK(intSave);
        return LOS_NOK;
    }

    node = (LosMemBoxNode *)(boxInfo + 1);

    boxInfo->stFreeList.pstNext = node;

    for (index = 0; index < boxInfo->blkNum - 1; ++index) {
        node->pstNext = OS_MEMBOX_NEXT(node, boxInfo->blkSize);
        node = node->pstNext;
    }

    node->pstNext = NULL;

#if (LOSCFG_PLATFORM_EXC == 1)
    OsMemBoxAdd(pool);
#endif

    MEMBOX_UNLOCK(intSave);

    return LOS_OK;
}

VOID *LOS_MemboxAlloc(VOID *pool)
{
    LosMemBoxInfo *boxInfo = (LosMemBoxInfo *)pool;
    LosMemBoxNode *node = NULL;
    LosMemBoxNode *nodeTmp = NULL;
    UINT32 intSave;

    if (pool == NULL) {
        return NULL;
    }

    MEMBOX_LOCK(intSave);
    node = &(boxInfo->stFreeList);
    if (node->pstNext != NULL) {
        nodeTmp = node->pstNext;
        node->pstNext = nodeTmp->pstNext;
        OsMemBoxSetMagic(nodeTmp);
        boxInfo->blkCnt++;
    }
    MEMBOX_UNLOCK(intSave);

    return (nodeTmp == NULL) ? NULL : OS_MEMBOX_USER_ADDR(nodeTmp);
}

UINT32 LOS_MemboxFree(VOID *pool, VOID *box)
{
    LosMemBoxInfo *boxInfo = (LosMemBoxInfo *)pool;
    UINT32 ret = LOS_NOK;
    UINT32 intSave;

    if ((pool == NULL) || (box == NULL)) {
        return LOS_NOK;
    }

    MEMBOX_LOCK(intSave);
    do {
        LosMemBoxNode *node = OS_MEMBOX_NODE_ADDR(box);
        if (OsCheckBoxMem(boxInfo, node) != LOS_OK) {
            break;
        }

        node->pstNext = boxInfo->stFreeList.pstNext;
        boxInfo->stFreeList.pstNext = node;
        boxInfo->blkCnt--;
        ret = LOS_OK;
    } while (0);
    MEMBOX_UNLOCK(intSave);

    return ret;
}

VOID LOS_MemboxClr(VOID *pool, VOID *box)
{
    LosMemBoxInfo *boxInfo = (LosMemBoxInfo *)pool;

    if ((pool == NULL) || (box == NULL)) {
        return;
    }

    (VOID)memset_s(box, (boxInfo->blkSize - OS_MEMBOX_NODE_HEAD_SIZE), 0,
                   (boxInfo->blkSize - OS_MEMBOX_NODE_HEAD_SIZE));
}

VOID LOS_ShowBox(VOID *pool)
{
    UINT32 index;
    UINT32 intSave;
    LosMemBoxInfo *boxInfo = (LosMemBoxInfo *)pool;
    LosMemBoxNode *node = NULL;

    if (pool == NULL) {
        return;
    }
    MEMBOX_LOCK(intSave);
    PRINT_INFO("membox(%p, 0x%x, 0x%x):\r\n", pool, boxInfo->blkSize, boxInfo->blkNum);
    PRINT_INFO("free node list:\r\n");

    for (node = boxInfo->stFreeList.pstNext, index = 0; node != NULL;
        node = node->pstNext, ++index) {
        PRINT_INFO("(%u, %p)\r\n", index, node);
    }

    PRINT_INFO("all node list:\r\n");
    node = (LosMemBoxNode *)(boxInfo + 1);
    for (index = 0; index < boxInfo->blkNum; ++index, node = OS_MEMBOX_NEXT(node, boxInfo->blkSize)) {
        PRINT_INFO("(%u, %p, %p)\r\n", index, node, node->pstNext);
    }
    MEMBOX_UNLOCK(intSave);
}

UINT32 LOS_MemboxStatisticsGet(const VOID *boxMem, UINT32 *maxBlk,
                               UINT32 *blkCnt, UINT32 *blkSize)
{
    if ((boxMem == NULL) || (maxBlk == NULL) || (blkCnt == NULL) || (blkSize == NULL)) {
        return LOS_NOK;
    }

    *maxBlk = ((OsMemBox *)boxMem)->blkNum;
    *blkCnt = ((OsMemBox *)boxMem)->blkCnt;
    *blkSize = ((OsMemBox *)boxMem)->blkSize;

    return LOS_OK;
}

#if (LOSCFG_PLATFORM_EXC == 1)
STATIC VOID OsMemboxExcInfoGetSub(const LosMemBoxInfo *pool, MemInfoCB *memExcInfo)
{
    LosMemBoxNode *node = NULL;
    UINTPTR poolStart, poolEnd;
    UINT32 index;
    UINT32 intSave;

    (VOID)memset_s(memExcInfo, sizeof(MemInfoCB), 0, sizeof(MemInfoCB));

    MEMBOX_LOCK(intSave);
    memExcInfo->type = MEM_MANG_MEMBOX;
    memExcInfo->startAddr = (UINTPTR)pool;
    memExcInfo->blockSize = pool->blkSize;
    memExcInfo->size = pool->blkNum; /* Block num */
    memExcInfo->free = pool->blkNum - pool->blkCnt;

    poolStart = (UINTPTR)pool;
    poolEnd = poolStart + pool->blkSize * pool->blkNum + sizeof(LosMemBoxInfo);
    node = (LosMemBoxNode *)(pool + 1);
    for (index = 0; index < pool->blkNum; ++index, node = OS_MEMBOX_NEXT(node, pool->blkSize)) {
        if (((UINTPTR)node < poolStart) || ((UINTPTR)node >= poolEnd)) {
            if (OsMemBoxCheckMagic(node)) {
                memExcInfo->errorAddr = (UINT32)(UINTPTR)((CHAR *)node + OS_MEMBOX_NODE_HEAD_SIZE);
                memExcInfo->errorLen = pool->blkSize - OS_MEMBOX_NODE_HEAD_SIZE;
                memExcInfo->errorOwner = OS_MEMBOX_TASKID_GET(node->pstNext);
                break;
            }
        }
    }
    MEMBOX_UNLOCK(intSave);
}

UINT32 OsMemboxExcInfoGet(UINT32 memNumMax, MemInfoCB *memExcInfo)
{
    LosMemBoxInfo *memBox = g_memBoxHead;
    UINT32 count = 0;
    UINT8 *buffer = (UINT8 *)memExcInfo;

    while (memBox != NULL) {
        OsMemboxExcInfoGetSub(memBox, (MemInfoCB *)buffer);
        count++;
        buffer += sizeof(MemInfoCB);
        if (count >= memNumMax) {
            break;
        }
        memBox = memBox->nextMemBox;
    }

    return count;
}
#endif


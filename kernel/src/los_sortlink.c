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

#include "los_sortlink.h"
#include "los_sched.h"
#include "los_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

STATIC SortLinkAttribute g_taskSortLink;
STATIC SortLinkAttribute g_swtmrSortLink;
STATIC BOOL g_isMiniPeriod = FALSE;
UINT32 g_tickMiniCount;

VOID OsSortLinkMiniPeriodEnable(VOID)
{
    g_isMiniPeriod = TRUE;
}

UINT32 OsSortLinkInit(SortLinkAttribute *sortLinkHeader)
{
    LOS_ListInit(&sortLinkHeader->sortLink);
    return LOS_OK;
}

STATIC INLINE VOID OsAddNode2SortLink(SortLinkAttribute *sortLinkHeader, SortLinkList *sortList)
{
    LOS_DL_LIST *head = (LOS_DL_LIST *)&sortLinkHeader->sortLink;

    if (LOS_ListEmpty(head)) {
        LOS_ListAdd(head, &sortList->sortLinkNode);
        return;
    }

    SortLinkList *listSorted = LOS_DL_LIST_ENTRY(head->pstNext, SortLinkList, sortLinkNode);
    if (listSorted->responseTime > sortList->responseTime) {
        LOS_ListAdd(head, &sortList->sortLinkNode);
        return;
    } else if (listSorted->responseTime == sortList->responseTime) {
        LOS_ListAdd(head->pstNext, &sortList->sortLinkNode);
        return;
    }

    LOS_DL_LIST *prevNode = head->pstPrev;
    do {
        listSorted = LOS_DL_LIST_ENTRY(prevNode, SortLinkList, sortLinkNode);
        if (listSorted->responseTime <= sortList->responseTime) {
            LOS_ListAdd(prevNode, &sortList->sortLinkNode);
            break;
        }

        prevNode = prevNode->pstPrev;
    } while (1);
}

STATIC INLINE VOID OsDeleteNodeSortLink(SortLinkAttribute *sortLinkHeader, SortLinkList *sortList)
{
    LOS_ListDelete(&sortList->sortLinkNode);
    SET_SORTLIST_VALUE(sortList, OS_SORT_LINK_INVALID_TIME);
}

STATIC INLINE UINT64 OsGetSortLinkNextExpireTime(SortLinkAttribute *sortHeader, UINT64 startTime)
{
    UINT64 expireTime = 0;
    UINT64 nextExpireTime = OS_SCHED_MAX_RESPONSE_TIME - OS_TICK_RESPONSE_PRECISION;
    LOS_DL_LIST *head = &sortHeader->sortLink;
    LOS_DL_LIST *list = head->pstNext;

    if (LOS_ListEmpty(head)) {
        return nextExpireTime;
    }

    do {
        SortLinkList *listSorted = LOS_DL_LIST_ENTRY(list, SortLinkList, sortLinkNode);
        if (listSorted->responseTime <= (startTime + OS_TICK_RESPONSE_PRECISION)) {
            expireTime = listSorted->responseTime;
            list = list->pstNext;
        } else {
            nextExpireTime = listSorted->responseTime;
            break;
        }
    } while (list != head);

    if (expireTime >= startTime) {
        return expireTime;
    }

    /* An existing node times out */
    if (expireTime) {
        return startTime;
    }

    return nextExpireTime;
}

VOID OsAddTask2SortLink(SortLinkList *node, UINT64 startTime, UINT32 waitTicks)
{
    UINT32 intSave = LOS_IntLock();
    SET_SORTLIST_VALUE(node, startTime + (UINT64)waitTicks * OS_CYCLE_PER_TICK);
    OsAddNode2SortLink(&g_taskSortLink, node);
    LOS_IntRestore(intSave);
}

VOID OsDeleteTaskFromSortLink(SortLinkList *node)
{
    UINT32 intSave = LOS_IntLock();
    if (node->responseTime != OS_SORT_LINK_INVALID_TIME) {
        OsDeleteNodeSortLink(&g_taskSortLink, node);
    }
    LOS_IntRestore(intSave);
}

VOID OsAddSwtmr2SortLink(SortLinkList *node, UINT64 startTime, UINT64 waitTime, UINT32 ticks)
{
    UINT32 intSave = LOS_IntLock();
    SET_SORTLIST_VALUE(node, startTime + waitTime);
    OsAddNode2SortLink(&g_swtmrSortLink, node);
    if (g_isMiniPeriod && (ticks == OS_TICK_MINI_PERIOD)) {
        g_tickMiniCount++;
    }
    LOS_IntRestore(intSave);
}

VOID OsDeleteSwtmrFromSortLink(SortLinkList *node, UINT32 ticks)
{
    UINT32 intSave = LOS_IntLock();
    if (node->responseTime != OS_SORT_LINK_INVALID_TIME) {
        OsDeleteNodeSortLink(&g_swtmrSortLink, node);
        if (g_isMiniPeriod && (ticks == OS_TICK_MINI_PERIOD)) {
            g_tickMiniCount--;
        }
    }
    LOS_IntRestore(intSave);
}

SortLinkAttribute *OsGetSortLinkAttribute(SortLinkType type)
{
    if (type == OS_SORT_LINK_TASK) {
        return &g_taskSortLink;
    } else if (type == OS_SORT_LINK_SWTMR) {
        return &g_swtmrSortLink;
    }

    PRINT_ERR("Invalid sort link type!\n");
    return NULL;
}

UINT64 OsGetNextExpireTime(UINT64 startTime)
{
    UINT32 intSave;
    SortLinkAttribute *taskHeader = &g_taskSortLink;
    SortLinkAttribute *swtmrHeader = &g_swtmrSortLink;

    intSave = LOS_IntLock();
    UINT64 taskExpirTime = OsGetSortLinkNextExpireTime(taskHeader, startTime);
    UINT64 swtmrExpirTime = OsGetSortLinkNextExpireTime(swtmrHeader, startTime);
    LOS_IntRestore(intSave);

    return (taskExpirTime < swtmrExpirTime) ? taskExpirTime : swtmrExpirTime;
}

UINT32 OsSortLinkGetTargetRemainTimeTick(UINT64 currTime, const SortLinkList *targetSortList)
{
    if (currTime >= targetSortList->responseTime) {
        return 0;
    }

    return (UINT32)((targetSortList->responseTime - currTime) / OS_CYCLE_PER_TICK);
}

UINT32 OsSortLinkGetNextExpireTimeTick(const SortLinkAttribute *sortLinkHeader)
{
    LOS_DL_LIST *head = (LOS_DL_LIST *)&sortLinkHeader->sortLink;

    if (LOS_ListEmpty(head)) {
        return 0;
    }

    SortLinkList *listSorted = LOS_DL_LIST_ENTRY(head->pstNext, SortLinkList, sortLinkNode);
    return OsSortLinkGetTargetRemainTimeTick(OsGetCurrSchedTimeCycle(), listSorted);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

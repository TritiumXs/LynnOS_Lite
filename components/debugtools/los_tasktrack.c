/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_debugtools.h"
#include "securec.h"
#include "los_debug.h"
#include "los_task.h"
#include "los_memory.h"
#include "los_arch.h"

#if (LOSCFG_DEBUG_TOOLS == 1)

#define TRACE_NUM 200
typedef struct {
    UINT32 taskID;
    UINT32 taskConut;
    CHAR taskName[LOS_TASK_NAMELEN];
} TaskTrackInfo;

STATIC BOOL g_startTrace = FALSE;
STATIC TaskTrackInfo *g_traceRingBuf = NULL;
STATIC UINT32 g_trackCount = 0;

STATIC VOID ShowFormat(VOID)
{
    INT32 i;
    UINT32 cycle = (g_trackCount - 1) / TRACE_NUM;
    UINT32 point = g_trackCount % TRACE_NUM;

    if (g_trackCount == 0) {
        PRINT_ERR("none shed happend\n");
        return;
    }

    PRINTK ("in 1s, sched %d time, last %d is:\r\n", g_trackCount - 1, TRACE_NUM);
    PRINTK("TASKID  SCHEDCOUNT       TASKNAME\r\n");
    if (cycle > 0) {
        for (i = point; i < TRACE_NUM; i++) {
            PRINTK("%6u %10u %20s\n", g_traceRingBuf[i].taskID, g_traceRingBuf[i].taskConut, g_traceRingBuf[i].taskName);
        }
    }

    PRINTK("TASKID  SCHEDCOUNT       TASKNAME  (part 2)\r\n");
    for (i = 0; i < point; i++) {
        PRINTK("%6u %10u %20s\n", g_traceRingBuf[i].taskID, g_traceRingBuf[i].taskConut, g_traceRingBuf[i].taskName);
    }

    PRINTK("\r\n");
}

VOID OsTaskTraceRecord(void *task)
{
    LosTaskCB *tp = NULL;
    UINT32 point = g_trackCount % TRACE_NUM;

    if ((g_startTrace == FALSE) || (task == NULL)) {
        return;
    }

    tp = (LosTaskCB *)task;

    //todo, add count ?
    //g_traceRingBuf[point].taskConut++;
    g_traceRingBuf[point].taskID = tp->taskID;
    (VOID)memcpy_s(g_traceRingBuf[point].taskName, LOS_TASK_NAMELEN, tp->taskName, LOS_TASK_NAMELEN);
    g_trackCount++;
}

VOID LOS_TaskTrack(UINT32 time)
{
    (void)time;

    g_traceRingBuf = (TaskTrackInfo *)LOS_MemAlloc(OS_SYS_MEM_ADDR, TRACE_NUM * sizeof(TaskTrackInfo));
    if (g_traceRingBuf == NULL) {
        PRINT_ERR("alloc failed for dump\n");
        return;
    }
    (VOID)memset_s(g_traceRingBuf, TRACE_NUM * sizeof(TaskTrackInfo), 0, TRACE_NUM * sizeof(TaskTrackInfo));

    g_startTrace = TRUE;

    //todo: need ms to tick?
    LOS_TaskDelay(1000);

    g_startTrace = FALSE;

    ShowFormat();
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, g_traceRingBuf);
    g_traceRingBuf = NULL;
    g_trackCount = 0;

    return;
}

UINT32 OsShellCmdTaskTrack(INT32 argc, const CHAR **argv)
{
    UINT32 time;

    if (argc != 1) {
        PRINT_ERR("\nUsage: tt time(ms)\n");
        return LOS_NOK;
    }

    time = (UINT32)atoi(argv[0]);

    LOS_TaskTrack(time);
    return LOS_OK;
}
#endif /* LOSCFG_STACK_DUMP == 1 */

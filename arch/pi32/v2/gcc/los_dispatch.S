/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
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

.section .text

// 关闭中断，并返回旧的 icfg
    .type ArchIntLock, @function
    .global ArchIntLock
ArchIntLock:
    cli r0
    rts
.L_endArchIntLock:
    .size ArchIntLock, . - .L_endArchIntLock

// 开中断，并返回旧的 icfg
    .type ArchIntUnLock, @function
    .global ArchIntUnLock
ArchIntUnLock:
    r0 = icfg
    sti
    rts

.L_endArchIntUnLock:
    .size ArchIntUnLock, . - .L_endArchIntUnLock

// 恢复 icfg
    .type ArchIntRestore, @function
    .global ArchIntRestore
ArchIntRestore:
    icfg = r0
    rts
.L_endArchIntRestore:
    .size ArchIntRestore, . - .L_endArchIntRestore




    .type ArchPendSV, @function
    .global ArchPendSV
ArchPendSV:
// 发生中断的时候，GIE0 是为 0，表示中断关闭
    cli
    lockset
    csync
    csync
    csync
    csync
    csync





    [--sp] = {r15-r0}
    [--sp] = {psr,rets,reti}
    call sp_ovf_unen
    {psr,rets,reti} = [sp++]
    {r15-r0} = [sp++]

    sp = usp
    [--sp] = {r15-r0}
    [--sp] = {psr,rets,reti}







// store sp to task stack
    r0 = g_losTask
    r0 = [r0 + 0]
    r1 = sp
    [r0 + 0] = r1

    call ArchTaskSwitch

// 如果无需切换任务，则返回
    if (r0 == 0) goto 1f

// load new task
    r0 = g_losTask
    r0 = [r0 + 0]
    r1 = [r0 + 0]
    sp = r1

// pop context and return

1:

    call ArchClearSchedulePending
    {psr,rets,reti} = [sp++]
    {r15-r0} = [sp++]
    usp = sp
    sp = ssp

    [--sp] = {r15-r0}
    [--sp] = {psr,rets,reti}
    call sp_ovf_en
    {psr,rets,reti} = [sp++]
    {r15-r0} = [sp++]

    lockclr
    sti
    rti


    .type ArchStartToRun, @function
    .global ArchStartToRun
ArchStartToRun:
//     LosTaskCB   *runTask;
//     LosTaskCB   *newTask;
    r1 = g_losTask
    r0 = [r1 + 4] // newTask

// retrieve stack pointer
    r0 = [r0 + 0]
    sp = r0

// pop out context
    {psr, rets, reti} = [sp++]
    {r15-r0} = [sp++]

    lockclr
    sti
    rti

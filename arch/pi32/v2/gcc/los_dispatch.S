/*
 *  Copyright (c) 2022 ZhuHai Jieli Technology Co.,Ltd.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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

#include "soc.h"

.global  HalEnableIRQ
.global  HalDisableIRQ
.global  HalIntLock
.global  HalIntUnLock
.global  HalIntRestore
.global  HalStartToRun
.global  HalTaskContextSwitch
.global  HalTaskContextSwitch1
.extern  __irq_stack_top
.section .interrupt.text

.macro PUSH_ALL_REG
    addi sp, sp, -(32 * REGBYTES)
    SREG  t6, 2 * REGBYTES(sp)
    SREG  t5, 3 * REGBYTES(sp)
    SREG  t4, 4 * REGBYTES(sp)
    SREG  t3, 5 * REGBYTES(sp)
    SREG  t2, 6 * REGBYTES(sp)
    SREG  t1, 7 * REGBYTES(sp)
    SREG  t0, 8 * REGBYTES(sp)
    SREG  s11, 9 * REGBYTES(sp)
    SREG  s10, 10 * REGBYTES(sp)
    SREG  s9, 11 * REGBYTES(sp)
    SREG  s8, 12 * REGBYTES(sp)
    SREG  s7, 13 * REGBYTES(sp)
    SREG  s6, 14 * REGBYTES(sp)
    SREG  s5, 15 * REGBYTES(sp)
    SREG  a7, 18 * REGBYTES(sp)
    SREG  a6, 19 * REGBYTES(sp)
    SREG  a5, 20 * REGBYTES(sp)
    SREG  a4, 21 * REGBYTES(sp)
    SREG  a3, 22 * REGBYTES(sp)
    SREG  a2, 23 * REGBYTES(sp)
    SREG  a1, 24 * REGBYTES(sp)
    SREG  a0, 25 * REGBYTES(sp)
    SREG  s4, 26 * REGBYTES(sp)
    SREG  s3, 27 * REGBYTES(sp)
    SREG  s2, 28 * REGBYTES(sp)
    SREG  s1, 29 * REGBYTES(sp)
    SREG  s0, 30 * REGBYTES(sp)
    SREG  ra, 31 * REGBYTES(sp)
.endm

.macro POP_ALL_REG
    LREG  t6, 2 * REGBYTES(sp)
    LREG  t5, 3 * REGBYTES(sp)
    LREG  t4, 4 * REGBYTES(sp)
    LREG  t3, 5 * REGBYTES(sp)
    LREG  t2, 6 * REGBYTES(sp)
    LREG  t1, 7 * REGBYTES(sp)
    LREG  t0, 8 * REGBYTES(sp)
    LREG  s11, 9 * REGBYTES(sp)
    LREG  s10, 10 * REGBYTES(sp)
    LREG  s9, 11 * REGBYTES(sp)
    LREG  s8, 12 * REGBYTES(sp)
    LREG  s7, 13 * REGBYTES(sp)
    LREG  s6, 14 * REGBYTES(sp)
    LREG  s5, 15 * REGBYTES(sp)
    LREG  a7, 18 * REGBYTES(sp)
    LREG  a6, 19 * REGBYTES(sp)
    LREG  a5, 20 * REGBYTES(sp)
    LREG  a4, 21 * REGBYTES(sp)
    LREG  a3, 22 * REGBYTES(sp)
    LREG  a2, 23 * REGBYTES(sp)
    LREG  a1, 24 * REGBYTES(sp)
    LREG  a0, 25 * REGBYTES(sp)
    LREG  s4, 26 * REGBYTES(sp)
    LREG  s3, 27 * REGBYTES(sp)
    LREG  s2, 28 * REGBYTES(sp)
    LREG  s1, 29 * REGBYTES(sp)
    LREG  s0, 30 * REGBYTES(sp)
    LREG  ra, 31 * REGBYTES(sp)
    addi  sp, sp, 32 * REGBYTES
.endm   

.global HalTaskContextSwitch1
HalTaskContextSwitch1:
    PUSH_ALL_REG

    SREG a0, 16 * REGBYTES(sp)
    SREG ra, 17 * REGBYTES(sp)

    la   a2, g_losTask
    lw   a0, 0(a2)
    sw   sp, TASK_CB_KERNEL_SP(a0)

    lw   a0, 4(a2)
    sw   a0, 0(a2)

    la   a2, g_losTask
    lw   a0, 4(a2)

// retireve stack pointer
    lw      sp, TASK_CB_KERNEL_SP(a0)
    POP_ALL_REG

    ret 

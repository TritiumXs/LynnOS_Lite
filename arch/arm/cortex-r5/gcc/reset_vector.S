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

    .equ CPSR_IRQ_DISABLE,  0x80
    .equ CPSR_FIQ_DISABLE,  0x40
    .equ CPSR_THUMB_ENABLE, 0x20
    .equ CPSR_USER_MODE,    0x10
    .equ CPSR_FIQ_MODE,     0x11
    .equ CPSR_IRQ_MODE,     0x12
    .equ CPSR_SVC_MODE,     0x13
    .equ CPSR_ABT_MODE,     0x17
    .equ CPSR_UNDEF_MODE,   0x1B

    .global __exc_stack_top
    .global __irq_stack_top
    .global __fiq_stack_top
    .global __svc_stack_top
    .global __abt_stack_top
    .global __undef_stack_top
    .global __exc_stack
    .global __irq_stack
    .global __fiq_stack
    .global __svc_stack
    .global __abt_stack
    .global __undef_stack

    .extern HalExceptFiqHdl
    .extern HalExceptAddrAbortHdl
    .extern HalExceptDataAbortHdl
    .extern HalExceptPrefetchAbortHdl
    .extern HalExceptSwiHdl
    .extern HalExceptUndefInstrHdl
    .extern HalExceptIrqHdl
    .extern _c_int00

    .code 32
    .text

    .section ".vectors", "ax"
    .global _vector_start

_vector_start:
    B     _c_int00
    B     HalExceptUndefInstrHdl
    B     HalExceptSwiHdl
    B     HalExceptPrefetchAbortHdl
    B     HalExceptDataAbortHdl
    B     HalExceptAddrAbortHdl
    B     HalExceptIrqHdl
    B     HalExceptFiqHdl

    .section ".vectorsHeap", "wa", %nobits
    .align 3
__exc_stack:
    .space  0x00001000
__exc_stack_top:

__svc_stack:
    .space  0x00001000
__svc_stack_top:

__fiq_stack:
    .space  0x00000800
__fiq_stack_top:

__irq_stack:
    .space  0x00000200
__irq_stack_top:

__abt_stack:
    .space  0x00000200
__abt_stack_top:

__undef_stack:
    .space  0x00000200
__undef_stack_top:


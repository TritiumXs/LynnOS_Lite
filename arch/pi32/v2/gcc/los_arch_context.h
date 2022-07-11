
#ifndef _LOS_ARCH_CONTEXT_H
#define _LOS_ARCH_CONTEXT_H

#include "los_config.h"
#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */
#endif /* __cplusplus */

typedef struct TagTskContext {
    UINT32 reti;
    UINT32 rets;
    UINT32 psr;

    // GENERNAL REGISTERS
    UINT32 r0;
    UINT32 r1;
    UINT32 r2;
    UINT32 r3;
    UINT32 r4;
    UINT32 r5;
    UINT32 r6;
    UINT32 r7;
    UINT32 r8;
    UINT32 r9;
    UINT32 r10;
    UINT32 r11;
    UINT32 r12;
    UINT32 r13;
    UINT32 r14;
    UINT32 r15;
} TaskContext;

#ifdef __cplusplus
#if __cplusplus
}
#endif /* ifdef __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_ARCH_CONTEXT_H */
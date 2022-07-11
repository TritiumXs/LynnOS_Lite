#ifndef _LOS_ARCH_INTERRUPT_H
#define _LOS_ARCH_INTERRUPT_H

#include "los_config.h"
#include "los_compiler.h"
#include "los_interrupt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/**
 * @ingroup los_hwi
 * Highest priority of a hardware interrupt.
 */
#define OS_HWI_PRIO_HIGHEST        7

/**
 * @ingroup los_hwi
 * Lowest priority of a hardware interrupt.
 */
#define OS_HWI_PRIO_LOWEST         0

#define OS_HWI_MAX_NUM                  64

/**
 * Maximum interrupt number.
 */
#define OS_HWI_MAX                      ((OS_HWI_MAX_NUM) - 1)


VOID ArchHwiInit();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

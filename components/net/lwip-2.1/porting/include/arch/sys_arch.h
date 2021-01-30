/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Description: adapter sys_arch.h for lwip
 * Author: l00369723
 * Create: December 5, 2020
 */

#ifndef _LWIP_PORTING_SYS_ARCH_H_
#define _LWIP_PORTING_SYS_ARCH_H_

#include <stdint.h>
#include "memory_pool.h"
#include "los_mux.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Mutex
 */
typedef uint32_t sys_mutex_t;

/**
 * Semaphore
 */
typedef uint32_t sys_sem_t;

/**
 * MessageBox
 */
typedef uint32_t sys_mbox_t;

/**
 * Protector
 */
typedef void *sys_prot_t;

/**
 * Thread
 */
typedef uint32_t sys_thread_t;

#ifdef __cplusplus
}
#endif

#endif /* _LWIP_PORTING_SYS_ARCH_H_ */

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Description: adapter cc.h for lwip
 * Author: l00369723
 * Create: December 5, 2020
 */

#ifndef _LWIP_PORTING_CC_H_
#define _LWIP_PORTING_CC_H_

#include <stdio.h>
#include <stdlib.h>
#include "securec.h"
#include "log.h"

#ifdef htons
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS
#endif

#define LWIP_PROVIDE_ERRNO 1
#define __SIZEOF_POINTER__ 4   // 32位系统

#define LOS_TASK_STATUS_DETACHED   0x0100  // 预留字段

#if defined(__arm__) && defined(__ARMCC_VERSION)
    /* Keil uVision4 tools */
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define ALIGNED(n)  __align(n)
#elif defined (__IAR_SYSTEMS_ICC__)
    /* IAR Embedded Workbench tools */
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    // #error NEEDS ALIGNED
#else
    /* GCC tools (CodeSourcery) */
    #define PACK_STRUCT_BEGIN
    #define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define ALIGNED(n)  __attribute__((aligned (n)))
#endif

#define LWIP_RAND rand

extern void HilogPrintf(const char *fmt, ...);

#define LWIP_PLATFORM_DIAG(vars) HilogPrintf vars
#define LWIP_PLATFORM_ASSERT(x) do {HILOG_ERROR(HILOG_MODULE_APP, \
                                    "Assertion \"%s\" errno %d line %d in %s\n", \
                                    x, errno, __LINE__, __FILE__);} while (0)

#define mem_clib_malloc LWIP_MEM_ALLOC
#define mem_clib_free LWIP_MEM_FREE
#define mem_clib_calloc LWIP_MEM_CALLOC

#define init_waitqueue_head(...)
#define poll_check_waiters(...)
#define IOCTL_CMD_CASE_HANDLER()
#define DF_NADDR(addr)

#define DNS_SERVER_ADDRESS(ipaddr)        (ip4_addr_set_u32(ipaddr, ipaddr_addr("114.114.114.114")))
#define DNS_SERVER_ADDRESS_SECONDARY(ipaddr)        (ip4_addr_set_u32(ipaddr, ipaddr_addr("114.114.115.115")))

#endif /* _LWIP_PORTING_CC_H_ */

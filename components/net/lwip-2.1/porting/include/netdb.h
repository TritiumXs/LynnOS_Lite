/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Description: used to provide dns interface
 * Author: l00369723
 * Create: December 5, 2020
 */

#ifndef __LITEOS_NETDB_PORTING_H__
#define __LITEOS_NETDB_PORTING_H__

#include "lwip/netdb.h"

struct hostent *gethostbyname(const char *name);

#endif // __LITEOS_NETDB_PORTING_H__
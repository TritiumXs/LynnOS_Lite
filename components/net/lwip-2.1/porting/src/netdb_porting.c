/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Description: dns porting and provide interface
 * Author: l00369723
 * Create: December 5, 2020
 */

#include "netdb.h"

struct hostent *gethostbyname(const char *name)
{
    if (name == NULL) {
        return NULL;
    }
    return lwip_gethostbyname(name);
}
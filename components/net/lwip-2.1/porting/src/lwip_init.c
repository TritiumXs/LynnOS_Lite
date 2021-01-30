/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Description: lwip init enter
 * Author: l00369723
 * Create: December 5, 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include "lwip/tcpip.h"
#include "ohos_init.h"
void TcpIpPortingInit()
{
    tcpip_init(NULL, NULL);
}
SYSEX_SERVICE_INIT(TcpIpPortingInit);

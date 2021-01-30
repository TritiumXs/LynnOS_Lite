/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Description: extend netif
 * Author: l00369723
 * Create: December 5, 2020
 */

#ifndef _LWIP_PORTING_NETIF_H_
#define _LWIP_PORTING_NETIF_H_

#define netif_find netifapi_netif_find_by_name

#if LWIP_DHCPS
#define LWIP_NETIF_CLIENT_DATA_INDEX_DHCP   LWIP_NETIF_CLIENT_DATA_INDEX_DHCP, \
                                            LWIP_NETIF_CLIENT_DATA_INDEX_DHCPS
#endif

#define LWIP_NETIF_FULLNAME 16
#define linkoutput      linkoutput; \
                        void (*drv_send)(struct netif *netif, struct pbuf *p); \
                        u8_t (*drv_set_hwaddr)(struct netif *netif, u8_t *addr, u8_t len); \
                        void (*drv_config)(struct netif *netif, u32_t config_flags, u8_t setBit); \
                        char full_name[LWIP_NETIF_FULLNAME]; \
                        u16_t link_layer_type
#include_next <lwip/netif.h>
#undef linkoutput
#if LWIP_DHCPS
#undef LWIP_NETIF_CLIENT_DATA_INDEX_DHCP
#endif

#include <lwip/etharp.h> // For ETHARP_HWADDR_LEN, by `hieth-sf src/interface.c' and `wal/wal_net.c'

#ifdef __cplusplus
extern "C" {
#endif

// redefine NETIF_NAMESIZE which was defined in netif.h
#undef NETIF_NAMESIZE
#define NETIF_NAMESIZE LWIP_NETIF_FULLNAME

#define LOOPBACK_IF         0 // 772
#define ETHERNET_DRIVER_IF  1
#define WIFI_DRIVER_IF      801
#define BT_PROXY_IF         802

err_t driverif_init(struct netif *netif);
void driverif_input(struct netif *netif, struct pbuf *p);

#ifdef __cplusplus
}
#endif

#endif /* _LWIP_PORTING_NETIF_H_ */

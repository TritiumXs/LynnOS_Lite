/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Description: extend netif api
 * Author: l00369723
 * Create: December 5, 2020
 */

#ifndef _LWIP_PORTING_NETIFAPI_H_
#define _LWIP_PORTING_NETIFAPI_H_

#include_next <lwip/netifapi.h>

#ifdef __cplusplus
extern "C" {
#endif

err_t netifapi_dhcps_start(struct netif *netif, char *start_ip, u16_t ip_num);
err_t netifapi_dhcps_stop(struct netif *netif);

#define netifapi_dhcp_cleanup(n)          netifapi_netif_common(n, dhcp_cleanup, NULL)
#define netifapi_dhcp_is_bound(n)         netifapi_netif_common(n, NULL, dhcp_is_bound)

void netifapi_netif_rmv_ip6_address(struct netif *netif, ip_addr_t *ipaddr);
struct netif *netifapi_netif_find_by_name(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* _LWIP_PORTING_NETIFAPI_H_ */

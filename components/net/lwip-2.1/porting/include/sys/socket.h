/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Description: used to provide socket interface
 * Author: l00369723
 * Create: December 5, 2020
 */

#ifndef	_SYS_SOCKET_PORTING_H
#define	_SYS_SOCKET_PORTING_H

#include "lwip/sockets.h"

#ifdef __cplusplus
extern "C" {
#endif

int socket (int, int, int);

int bind (int, const struct sockaddr *, socklen_t);
int connect (int, const struct sockaddr *, socklen_t);
int listen (int, int);
int accept (int, struct sockaddr *__restrict, socklen_t *__restrict);

int getsockname (int, struct sockaddr *__restrict, socklen_t *__restrict);
int getpeername (int, struct sockaddr *__restrict, socklen_t *__restrict);

ssize_t send (int, const void *, size_t, int);
ssize_t recv (int, void *, size_t, int);
ssize_t sendto (int, const void *, size_t, int, const struct sockaddr *, socklen_t);
ssize_t recvfrom (int, void *__restrict, size_t, int, struct sockaddr *__restrict, socklen_t *__restrict);
ssize_t sendmsg (int, const struct msghdr *, int);
ssize_t recvmsg (int, struct msghdr *, int);

int getsockopt (int, int, int, void *__restrict, socklen_t *__restrict);
int setsockopt (int, int, int, const void *, socklen_t);

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
int inet_pton(int af, const char *src, void *dst);

int shutdown (int, int);
int closesocket(int sockfd);

int ioctlsocket(int s, long cmd, void *argp);

#if LWIP_SOCKET_SELECT
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
#endif
#if LWIP_SOCKET_POLL
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
#endif

#ifdef __cplusplus
}
#endif
#endif

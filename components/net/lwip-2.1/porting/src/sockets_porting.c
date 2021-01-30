/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Description: socket porting and provide interface
 * Author: l00369723
 * Create: December 5, 2020
 */

#include "lwip/priv/sockets_priv.h"
#include <lwip/sockets.h>

#if !LWIP_COMPAT_SOCKETS

#define CHECK_NULL_PTR(ptr) do { if ((ptr) == NULL) { set_errno(EFAULT); return -1; } } while (0)

int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    return lwip_accept(s, addr, addrlen);
}

int bind(int s, const struct sockaddr *name, socklen_t namelen)
{
    CHECK_NULL_PTR(name);
    if (namelen < sizeof(*name)) {
        set_errno(EINVAL);
        return -1;
    }
    return lwip_bind(s, name, namelen);
}

int shutdown(int s, int how)
{
    return lwip_shutdown(s, how);
}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
    CHECK_NULL_PTR(name);
    CHECK_NULL_PTR(namelen);
    return lwip_getpeername(s, name, namelen);
}

int getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
    CHECK_NULL_PTR(name);
    CHECK_NULL_PTR(namelen);
    return lwip_getsockname(s, name, namelen);
}

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
{
    return lwip_getsockopt(s, level, optname, optval, optlen);
}

int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
    return lwip_setsockopt(s, level, optname, optval, optlen);
}

int closesocket(int s)
{
    return lwip_close(s);
}

int connect(int s, const struct sockaddr *name, socklen_t namelen)
{
    CHECK_NULL_PTR(name);
    if (namelen < sizeof(*name)) {
        set_errno(EINVAL);
        return -1;
    }
    return lwip_connect(s, name, namelen);
}

int listen(int s, int backlog)
{
    return lwip_listen(s, backlog);
}

ssize_t recv(int s, void *mem, size_t len, int flags)
{
    CHECK_NULL_PTR(mem);
    return lwip_recv(s, mem, len, flags);
}

ssize_t recvfrom(int s, void *mem, size_t len, int flags,
                 struct sockaddr *from, socklen_t *fromlen)
{
    CHECK_NULL_PTR(mem);
    return lwip_recvfrom(s, mem, len, flags, from, fromlen);
}

ssize_t recvmsg(int s, struct msghdr *message, int flags)
{
    CHECK_NULL_PTR(message);
    if (message->msg_iovlen) {
        CHECK_NULL_PTR(message->msg_iov);
    }
    return lwip_recvmsg(s, message, flags);
}

ssize_t send(int s, const void *dataptr, size_t size, int flags)
{
    CHECK_NULL_PTR(dataptr);
    return  lwip_send(s, dataptr, size, flags);
}

ssize_t sendmsg(int s, const struct msghdr *message, int flags)
{
    return lwip_sendmsg(s, message, flags);
}

ssize_t sendto(int s, const void *dataptr, size_t size, int flags,
               const struct sockaddr *to, socklen_t tolen)
{
    CHECK_NULL_PTR(dataptr);
    if (to && tolen < sizeof(*to)) {
        set_errno(EINVAL);
        return -1;
    }
    return lwip_sendto(s, dataptr, size, flags, to, tolen);
}

int socket(int domain, int type, int protocol)
{
    return lwip_socket(domain, type, protocol);
}

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    return lwip_inet_ntop(af, src, dst, size);
}

int inet_pton(int af, const char *src, void *dst)
{
    return lwip_inet_pton(af, src, dst);
}

int ioctlsocket(int s, long cmd, void *argp)
{
    return lwip_ioctl(s, cmd, argp);
}

#if LWIP_SOCKET_SELECT
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
    return lwip_select(maxfdp1, readset, writeset, exceptset, timeout);
}
#endif

#if LWIP_SOCKET_POLL
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    return lwip_poll(fds, nfds, timeout);
}
#endif

#endif /* !LWIP_COMPAT_SOCKETS */
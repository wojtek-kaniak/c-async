#if !defined (INCL_ASYNC_IO_SOCKET_H)
#define INCL_ASYNC_IO_SOCKET_H
#include <sys/socket.h>

int async_io_socket(int domain, int type, int protocol);

/// See accept4(2)
int async_io_accept(int fd, struct sockaddr* restrict addr, socklen_t* restrict addr_len, int flags);

int async_io_connect(int fd, const struct sockaddr* addr, socklen_t len);

#endif

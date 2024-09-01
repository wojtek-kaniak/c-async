#if !defined (INCL_ASYNC_IO_IO_H)
#define INCL_ASYNC_IO_IO_H

#include <inttypes.h> // IWYU pragma: keep

#include <sys/uio.h>
#include <sys/types.h>

void async_io_thread_init();

int async_io_openat(int dfd, const char *path, int flags, mode_t mode);

int async_io_close(int fd);

int async_io_preadv(int fd, const struct iovec* iovec, int count, uint64_t offset);

int async_io_pread(int fd, void* buf, size_t nbytes, uint64_t offset);

int async_io_pwritev(int fd, const struct iovec* iovecs, unsigned int nr_vecs, uint64_t offset);

int async_io_pwrite(int fd, const void* buf, unsigned int nbytes, uint64_t offset);

#endif

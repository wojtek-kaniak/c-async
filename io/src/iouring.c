#include <assert.h>

#include <liburing.h>
#include <stdint.h>

#include "async.h"
#include "bits/types/struct_iovec.h"
#include "common.h"

#include "async-io/io.h"
#include "async-io/socket.h"
#include "liburing/io_uring.h"

typedef struct IoUringOptions {
	bool poll_mode;
	uint queue_size;
} IoUringOptions;

static thread_local struct io_uring ring;

typedef struct Completion {
	i32 result;
	bool completed;
} Completion;

void async_io_iouring_initialize(IoUringOptions options)
{
	io_uring_queue_init(options.queue_size, &ring, 0);
}

static struct io_uring_sqe* new_sqe()
{
	struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);

	// TODO: handle null if full
	assert(sqe != nullptr);

	return sqe;
}

static void submit_sqe(struct io_uring_sqe* sqe, Completion* completion)
{
	completion->completed = false;
	io_uring_sqe_set_data(sqe, completion);

	io_uring_submit(&ring);
}

static void enqueue_preadv(Completion* completion, int fd, const struct iovec* iovec, int count, u64 offset)
{
	assert(offset != (uint64_t)-1);

	struct io_uring_sqe* sqe = new_sqe();

	io_uring_prep_readv(sqe, fd, iovec, count, offset);

	submit_sqe(sqe, completion);
}

static void enqueue_accept(Completion* completion, int fd, struct sockaddr* addr, socklen_t* addrlen, int flags)
{
	struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);

	// TODO: handle null if full
	assert(sqe != nullptr);

	io_uring_prep_accept(sqe, fd, addr, addrlen, flags);

	submit_sqe(sqe, completion);
}

void async_io_iouring_update()
{
	struct io_uring_cqe* cqe;

	while (io_uring_peek_cqe(&ring, &cqe) == 0)
	{
		Completion* completion = (void*)cqe->user_data;

		completion->result = cqe->res;
		completion->completed = true;

		io_uring_cqe_seen(&ring, cqe);
	}
}

void async_io_thread_init()
{
	async_io_iouring_initialize(
		(IoUringOptions) {
			.poll_mode = false,
			.queue_size = 1024,
		}
	);
}

// TODO: refactor into a generic await function with smarter scheduling?
static int await_for_completion(Completion* completion)
{
	async_io_iouring_update();
	while (!completion->completed)
	{
		async_yield();
		async_io_iouring_update();
	}

	return completion->result;
}

int async_io_openat(int dfd, const char* path, int flags, mode_t mode)
{
	Completion completion = {0};

	struct io_uring_sqe* sqe = new_sqe();

	io_uring_prep_openat(sqe, dfd, path, flags, mode);

	submit_sqe(sqe, &completion);

	return await_for_completion(&completion);
}

int async_io_close(int fd)
{
	Completion completion = {0};

	struct io_uring_sqe* sqe = new_sqe();

	io_uring_prep_close(sqe, fd);

	submit_sqe(sqe, &completion);

	return await_for_completion(&completion);
}

int async_io_preadv(int fd, const struct iovec *iovec, int count, uint64_t offset)
{
	Completion completion = {0};

	enqueue_preadv(&completion, fd, iovec, count, offset);

	return await_for_completion(&completion);
}

int async_io_pread(int fd, void* buf, size_t nbytes, uint64_t offset)
{
	Completion completion = {0};

	struct io_uring_sqe* sqe = new_sqe();

	io_uring_prep_read(sqe, fd, buf, nbytes, offset);

	submit_sqe(sqe, &completion);

	return await_for_completion(&completion);
}

int async_io_pwritev(int fd, const struct iovec* iovecs, unsigned int nr_vecs, uint64_t offset)
{
	Completion completion = {0};

	struct io_uring_sqe* sqe = new_sqe();

	io_uring_prep_writev(sqe, fd, iovecs, nr_vecs, offset);

	submit_sqe(sqe, &completion);

	return await_for_completion(&completion);
}

int async_io_pwrite(int fd, const void* buf, unsigned int nbytes, uint64_t offset)
{
	Completion completion = {0};

	struct io_uring_sqe* sqe = new_sqe();

	io_uring_prep_write(sqe, fd, buf, nbytes, offset);

	submit_sqe(sqe, &completion);

	return await_for_completion(&completion);
}

int async_io_accept(int fd, struct sockaddr* restrict addr, socklen_t* restrict addr_len, int flags)
{
	Completion completion = {0};

	enqueue_accept(&completion, fd, addr, addr_len, flags);

	return await_for_completion(&completion);
}

int async_io_socket(int domain, int type, int protocol)
{
	Completion completion = {0};

	struct io_uring_sqe* sqe = new_sqe();

	io_uring_prep_socket(sqe, domain, type, protocol, 0);

	submit_sqe(sqe, &completion);

	return await_for_completion(&completion);
}

int async_io_connect(int fd, const struct sockaddr* addr, socklen_t len)
{
	Completion completion = {0};

	struct io_uring_sqe* sqe = new_sqe();

	io_uring_prep_connect(sqe, fd, addr, len);

	submit_sqe(sqe, &completion);

	return await_for_completion(&completion);
}

void async_io_iouring_free()
{
	io_uring_queue_exit(&ring);
}

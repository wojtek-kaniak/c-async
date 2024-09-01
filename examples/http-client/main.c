#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <async.h>
#include <async-io/io.h>
#include <async-io/socket.h>

void download_page(uintptr_t data);

void store_page(int dir_base_fd, int socket_fd);

int resolve_hostname(const char* name, struct addrinfo* resolved_address);

bool str_starts_with(const char* string, const char* prefix);

static int base_save_dir_fd;

static int file_name_counter = 0;

int main(int argc, const char** argv)
{
	async_io_thread_init();

	async_runtime_create();

	base_save_dir_fd = open(".", O_PATH);

	assert(base_save_dir_fd > 0);

	assert(argc > 1);

	for (size_t i = 1; i < argc; i++)
	{
		async_spawn(download_page, (uintptr_t)argv[i]);
	}

	async_runtime_start();
}

void download_page(uintptr_t data)
{
	const char HTTP_SCHEMA[] = "http://";

	const char* uri = (const char*)data;

	if (!str_starts_with(uri, HTTP_SCHEMA))
	{
		fprintf(stderr, "invalid URI schema: '%s'\n", uri);
		return;
	}

	const char* uri_no_schema = uri + sizeof(HTTP_SCHEMA) - 1;

	size_t origin_len;

	const char* uri_path = strchr(uri_no_schema, '/');

	if (uri_path == nullptr)
	{
		// Normalize http://example.org to http://example.org/
		uri_path = "/";
		origin_len = strlen(uri_no_schema);
	}
	else
		origin_len = uri_path - uri_no_schema;

	if (origin_len == 0)
	{
		fprintf(stderr, "invalid URI: '%s'\n", uri);
		return;
	}

	char* origin = strndup(uri_no_schema, origin_len);

	assert(origin != nullptr);

	struct addrinfo address;
	int resolv_res = resolve_hostname(origin, &address);
	
	if (resolv_res < 0)
	{
		fprintf(stderr, "hostname resolution failed: '%s', error: %d\n", uri, resolv_res);
		return;
	}

	int socket = async_io_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	assert(socket > 0);

	int connect_res = async_io_connect(socket, address.ai_addr, address.ai_addrlen);

	if (connect_res == -ECONNREFUSED)
	{
		fprintf(stderr, "connection refused\n");
		return;
	}

	assert(connect_res == 0);

#define CRLF "\r\n"
#define HTTP_IOVEC(chars) { .iov_base = (void*)chars, .iov_len = sizeof(chars) - 1, }

	const struct iovec write_iovecs[] = {
		HTTP_IOVEC("GET "),
		{ .iov_base = (void*)uri_path, .iov_len = strlen(uri_path) },
		HTTP_IOVEC(
			" HTTP/1.1" CRLF
			"Host: "
		),
		{ .iov_base = (void*)origin, .iov_len = origin_len },
		HTTP_IOVEC(
			CRLF
			"Connection: close" CRLF
			"User-Agent: c-async-example-http-client" CRLF
			"Accept: text/html" CRLF
			CRLF
		),
	};
	async_io_pwritev(socket, write_iovecs, sizeof(write_iovecs) / sizeof(write_iovecs[0]), 0);

	store_page(base_save_dir_fd, socket);

	async_io_close(socket);
}

void store_page(int dir_base_fd, int socket_fd)
{
#define FILENAME_MAX_SIZE 32
	char filename[FILENAME_MAX_SIZE] = {0};
	snprintf(filename, FILENAME_MAX_SIZE - 1, "page-%d", file_name_counter++);

	int file_fd = async_io_openat(dir_base_fd, filename, O_CREAT | O_WRONLY | O_TRUNC, 0640);

	if (file_fd < 0)
	{
		errno = -file_fd;
		perror("openat");
		return;
	}

#define BUFFER_SIZE 4096
	char buffer[BUFFER_SIZE];
	size_t bytes_written = 0;

	while (true)
	{
		int read_res = async_io_pread(socket_fd, buffer, BUFFER_SIZE, 0);

		if (read_res < 0)
		{
			errno = -read_res;
			perror("pread");
			return;
		}
		else if (read_res == 0)
			break;
		else
		{
			write:
			int write_res = async_io_pwrite(file_fd, buffer, read_res, bytes_written);

			if (write_res < 0)
			{
				errno = -write_res;
				perror("pwrite");
				return;
			}

			bytes_written += write_res;

			if (write_res < read_res)
			{
				// In an event of a partial write the call may be repeated,
				// the next invocation will either complete the write or return an error.
				goto write;
			}
		}
	}

	async_io_close(file_fd);
#undef BUFFER_SIZE
}

int resolve_hostname(const char* name, struct addrinfo* resolved_address)
{
	const char* const PORT = "80";

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	// IPv4 only
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo* result_addrinfo;

	int res = getaddrinfo(name, PORT, &hints, &result_addrinfo);

	if (res < 0)
		return res;
	
	// Return the first matching addrinfo
	*resolved_address = *result_addrinfo;

	freeaddrinfo(result_addrinfo);

	return 0;
}

bool str_starts_with(const char* string, const char* prefix)
{
	return strncmp(string, prefix, strlen(prefix)) == 0;
}

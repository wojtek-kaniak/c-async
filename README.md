# c-async
A single-threaded (non work-stealing) coroutine runtime for C23.

> [!CAUTION]
> Not production safe, just a proof of concept.

# Supported platforms
Currently only x86-64 platforms with Sys-V ABI support.

# Components
- [**c-async**](src) - base coroutines support
- [**c-async-io**](io) - Linux async IO using iouring

# Usage
See the [examples](examples).

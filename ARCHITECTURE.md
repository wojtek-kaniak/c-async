# Architecture

C23 required, tested on clang 18

- [`src`](src) - c-async source
	- [`include/async.h`](src/include/async.h) - public API
	- [`arch`](src/arch) - target (ABI) specific code

- [`io`](io) - c-async-io source
	- [`include`](io/src/include) - public API

- [`common`](common) - common private headers (generic data structure definitions)

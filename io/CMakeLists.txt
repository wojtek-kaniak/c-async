file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE HEADERS src/*.h)

add_library(c-async-io ${SOURCES} ${HEADERS})

target_link_libraries(c-async-io PRIVATE c-async-headers uring)

target_include_directories(c-async-io PRIVATE src)

target_compile_definitions(c-async-io PRIVATE _DEFAULT_SOURCE)

target_compile_options(c-async-io PRIVATE -Wall -Wextra)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_options(c-async-io PRIVATE $<$<COMPILE_LANGUAGE:C>:-O0 -g3 -fsanitize=undefined -fsanitize=memory>)
	target_link_options(c-async-io PUBLIC -g3 -fsanitize=undefined -fsanitize=memory)
endif()

add_library(c-async-io-headers INTERFACE ${HEADERS})
target_include_directories(c-async-io-headers INTERFACE src/include)

target_link_libraries(c-async-io PUBLIC c-async-io-headers)

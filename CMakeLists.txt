cmake_minimum_required(VERSION 3.10)

project(c-async)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF)

enable_language(ASM_NASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)

add_compile_options(-Wall)

file(GLOB_RECURSE API_HEADERS src/include/*h)
add_library(c-async-headers INTERFACE ${API_HEADERS})
target_include_directories(c-async-headers INTERFACE src/include)

file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE ASM_SOURCES src/*.asm)
file(GLOB_RECURSE HEADERS src/*.h)
add_library(c-async ${SOURCES} ${ASM_SOURCES} ${HEADERS})

target_compile_options(c-async PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wextra>)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_options(c-async PUBLIC $<$<COMPILE_LANGUAGE:C>:-O0 -g3 -fsanitize=undefined -fsanitize=memory>)
	target_link_options(c-async PUBLIC -g3 -fsanitize=undefined -fsanitize=memory)
endif()

target_include_directories(c-async PRIVATE src)
target_link_libraries(c-async PRIVATE c-async-common)
target_link_libraries(c-async PUBLIC c-async-headers)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(c-async PRIVATE DEBUG=1)
endif()

option(CASYNC_VERBOSE "c-async: verbose logging" OFF)

if (CASYNC_VERBOSE)
	target_compile_definitions(c-async PRIVATE CASYNC_VERBOSE=1)
endif()

add_subdirectory(common)
add_subdirectory(io)
add_subdirectory(examples)

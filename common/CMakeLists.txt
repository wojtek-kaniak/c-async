file(GLOB_RECURSE COMMON_HEADERS *.h)
add_library(c-async-common INTERFACE ${COMMON_HEADERS})
target_include_directories(c-async-common INTERFACE .)

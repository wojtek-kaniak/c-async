file(GLOB_RECURSE EXAMPLE_HELLOWORLD_SOURCES *.c)
file(GLOB_RECURSE EXAMPLE_HELLOWORLD_HEADERS *.h)
add_executable(c-async-example-helloworld ${EXAMPLE_HELLOWORLD_SOURCES} ${EXAMPLE_HELLOWORLD_HEADERS})

target_link_libraries(c-async-example-helloworld c-async)

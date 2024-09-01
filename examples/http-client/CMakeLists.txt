file(GLOB_RECURSE EXAMPLE_HTTP_CLIENT_SOURCES *.c)
file(GLOB_RECURSE EXAMPLE_HTTP_CLIENT_HEADERS *.h)
add_executable(c-async-example-http-client ${EXAMPLE_HTTP_CLIENT_SOURCES} ${EXAMPLE_HTTP_CLIENT_HEADERS})

target_link_libraries(c-async-example-http-client c-async)
target_link_libraries(c-async-example-http-client c-async-io)

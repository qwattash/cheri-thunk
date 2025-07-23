
find_package(Threads REQUIRED)

add_library(hello_thunk hello/hello.c hello/hello.S)

add_executable(test_thunk_core test_thunk_core.c test_malloc.c)
target_link_libraries(test_thunk_core Threads::Threads hello_thunk ${PROJECT_NAME})
add_test(NAME thunk-core COMMAND test_thunk_core)

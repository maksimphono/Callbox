cmake_minimum_required(VERSION 3.12)
project(SyscallBox LANGUAGES C)

add_executable(main
    src/executors.c
    src/print_message.c
    src/readers.c
    src/sandbox.c
    src/utils.c
    src/hashmap.c
    src/main.c
)

add_subdirectory(lib/tokenize)

target_link_libraries(main tokenize)

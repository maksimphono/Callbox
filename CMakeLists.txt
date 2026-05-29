cmake_minimum_required(VERSION 3.12)
project(SyscallBox LANGUAGES C)

# Regular build:

add_executable(main
    src/cli_arguments.c
    src/argparse.c
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

# Memory debug:

add_executable(main_mem
    src/argparse.c
    src/executors.c
    src/print_message.c
    src/readers.c
    src/sandbox.c
    src/utils.c
    src/hashmap.c
    src/main.c
)

target_link_libraries(main_mem tokenize)

target_compile_options(main_mem PRIVATE -Wall)
target_compile_options(main_mem PRIVATE -Wextra)
target_compile_options(main_mem PRIVATE -g)

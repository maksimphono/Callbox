cmake_minimum_required(VERSION 3.12)
project(SyscallBox LANGUAGES C)

add_executable(main
    buildins.c
    executors.c
    print_message.c
    readers.c
    sandbox.c
    utils.c
    hashmap.c
    main.c
)

# add_subdirectory(lib/tokenize)

# target_link_libraries(esh tokenize)

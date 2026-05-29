#include "../include/print_message.h"

void print_invalid_syntax() {
    printf("Invalid Syntax\n");
    fflush(stdout);
}

void print_execution_error() {
    printf("Execution Error\n");
    fflush(stdout);
}

void print_catched_syscall(const char* action, char* syscall_name, int count, ...) {
    va_list args;
    va_start(args, count);
    printf("%s Syscall: %s ", action, syscall_name);

    for (int i = 0; i < count; i++) {
        char* arg = va_arg(args, char*);
        printf("%s ", arg);
    }
    printf("\n");
    fflush(stdout);
}

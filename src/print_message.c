#include "../include/print_message.h"

void print_missing_tracee() {
    printf("Error: Traced program is missing, please specify program to trace after '--'\n");
    fflush(stdout);
}

void print_invalid_syntax() {
    printf("Invalid Syntax\n");
    fflush(stdout);
}

void print_execution_error() {
    printf("Execution Error\n");
    fflush(stdout);
}

void print_empty_rules() {
    printf("Warning: rules file wan't specified, tracing will be ineffetive. You can specify path to the rules file with argument '--rules'\n");
    fflush(stdout);
}

void print_catched_syscall(int fd, const char* action, char* syscall_name, int count, ...) {
    va_list args;
    va_start(args, count);
    dprintf(fd, "%s Syscall: %s ", action, syscall_name);

    for (int i = 0; i < count; i++) {
        char* arg = va_arg(args, char*);
        dprintf(fd, "%s ", arg);
    }
    dprintf(fd, "\n");
    //fflush(fd);
}

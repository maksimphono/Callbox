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

void print_single_syscall_argument(int fd, Syscall_argument* arg){
    if (arg->type == ARRAY_TYPE) {
        for (size_t i = 0; i < arg->arr_len; ++i) {
            if (isprint(arg->arr[i])) {
                dprintf(fd, "%c", arg->arr[i]);
            } else {
                switch (arg->arr[i]) {
                case '\n': dprintf(fd, "\\n"); break;
                case '\t': dprintf(fd, "\\t"); break;
                case '\0': dprintf(fd, "\\0"); break;
                case '\r': dprintf(fd, "\\r"); break;
                default: dprintf(fd, "\\x%02x", arg->arr[i]);
                }
            }
        
        }

        return;
    }

    const char* syscall_args_print_formats[] = {NULL, "%d", "%u", "%lld", "%llu", "\"%s\"", "%p", "0x%llx", "0x%llx", "0x%llx"};
    const char* fmt = syscall_args_print_formats[arg->type];

    switch (arg->type) {
    case UINT_32_TYPE: { dprintf(fd, fmt, arg->uint32);  break; }
    case UINT_64_TYPE: { dprintf(fd, fmt, arg->uint64);  break; }
    case INT_64_TYPE:  { dprintf(fd, fmt, arg->int64);   break; }
    case INT_32_TYPE:  { dprintf(fd, fmt, arg->int32);   break; }
    case STRING_TYPE:  { dprintf(fd, fmt, arg->str);     break; }
    case ADDRESS_TYPE: { dprintf(fd, fmt, arg->addr[0]); break; }
    }
}

void print_catched_syscall(int fd, const char* action, char* syscall_name, u_int8_t count, Syscall_argument args[MAX_SYSCALL_ARGS_NUM]) {
    dprintf(fd, "%s Syscall: %s ", action, syscall_name);

    for (int i = 0; args[i + 1].type != ___NONE_TYPE && i < count - 1; ++i) {
        print_single_syscall_argument(fd, &args[i]);
        dprintf(fd, ", ");
    }
    print_single_syscall_argument(fd, &args[count - 1]);
    dprintf(fd, "\n");
    //fflush(fd);
}

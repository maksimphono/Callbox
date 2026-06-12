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


void print_byte(int fd, byte_t byte) {
    if (isprint(byte)) {
        dprintf(fd, "'%c'", byte);
    } else {
        switch (byte) {
        case '\n': dprintf(fd, "'\\n'"); break;
        case '\t': dprintf(fd, "'\\t'"); break;
        case '\0': dprintf(fd, "'\\0'"); break;
        case '\r': dprintf(fd, "'\\r'"); break;
        default: dprintf(fd, "\\x%02x", byte);
        }
    }
}

void print_single_syscall_argument(int fd, Syscall_argument* arg){
    if (arg->type == ARRAY_TYPE) {
        size_t i = 0;
        dprintf(fd, "{ ");
        for (; i < arg->arr_len - 1; i++) {
            print_byte(fd, arg->arr[i]);
            dprintf(fd, ", ");
        }
        print_byte(fd, arg->arr[i]);
        dprintf(fd, " }");

        return;
    }

    const char* syscall_args_print_formats[] = {NULL, "%d", "%u", "%lld", "%llu", "\"%s\"", "%p", "0x%llx", "0x%llx", "0x%llx"};
    const char* fmt = syscall_args_print_formats[arg->type];

    switch (arg->type) {
    case UINT_TYPE:     { dprintf(fd, fmt, arg->uint);    break; }
    case INT_TYPE:      { dprintf(fd, fmt, arg->int_);    break; }
    case ULLONG_TYPE:   { dprintf(fd, fmt, arg->ullong);  break; }
    case LLONG_TYPE:    { dprintf(fd, fmt, arg->llong);   break; }
    case ULONG_TYPE:    { dprintf(fd, fmt, arg->ulong);   break; }
    case LONG_TYPE:     { dprintf(fd, fmt, arg->long_);   break; }
    case ADDRESS_TYPE:  { dprintf(fd, fmt, arg->addr[0]); break; }
    case STRING_TYPE:  { 
        dprintf(fd, "\"");
        for (size_t i = 0; arg->str[i] != '\0'; i++) {
            if (isprint(arg->str[i])) {
                dprintf(fd, "%c", arg->str[i]);
            } else {
                switch (arg->str[i]) {
                case '\n': dprintf(fd, "\\n"); break;
                case '\t': dprintf(fd, "\\t"); break;
                case '\r': dprintf(fd, "\\r"); break;
                default: dprintf(fd, "?");
                }
            }
            
        }
        dprintf(fd, "\"");
        break; 
    }
    
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

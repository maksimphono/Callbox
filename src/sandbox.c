#include "../include/sandbox.h"
#include "../include/hashmap.h"
#include "../include/syscalls_table.h"

Syscall_hashmap_t* syscalls_map;
Node_recorded_rules_t* syscalls_with_rules = NULL;
const Syscall_argument EMPTY_RULES = {___NONE_TYPE, 0};

Syscall_argument* get_rules_for_syscall_id(u_int32_t id){
    if (id > syscalls_table_len) return NULL;
    return syscalls_table[id].rules;
}
const char* get_name_for_syscall_id(u_int32_t id){
    if (id > syscalls_table_len) return NULL;
    return syscalls_table[id].name;
}

const Syscall_arg_type* get_syscall_argument_types(reg_t syscall_num) {
    if (syscall_num > syscalls_table_len) return NULL;
    return &syscalls_table[syscall_num].arg_types[0];
}

void init_syscall_rules() {
    //memset(&Syscall_rules, 0, NUMBER_OF_SYSCALLS * sizeof(char**));
    printf("init_syscall_rules\n");
    syscalls_map = new_Syscall_hashmap(NUMBER_OF_SYSCALLS);

    // fill hashmap [syscall_name:syscall_number] with values
    for (u_int32_t i = 0; i < syscalls_table_len; i++) {
        if (not(IS_EMPTY_SYSCALL(syscalls_table[i]))) {
            insert_Syscall_hashmap(syscalls_map, syscalls_table[i].name, syscalls_table[i].number);
        }
    }
    return;
}

void del_syscall_rules() {
    clean_syscall_rules();
    del_Syscall_hashmap(syscalls_map);
}

void clean_syscall_rules(){
    Node_recorded_rules_t* current = syscalls_with_rules, *next;

    while (current != NULL) {
        Syscall_argument* rule = syscalls_table[current->syscall_num].rules; // list of rules
        if (rule != NULL && rule != &EMPTY_RULES) {
            for (int i = 0; rule[i].type != ___NONE_TYPE; i++) {
                if (rule[i].type == STRING_TYPE)
                    free(rule[i].str);
                else if (rule[i].type == ARRAY_TYPE)
                    free(rule[i].arr);
                //free(rule[i]);
            }
            free(rule);
        }
        syscalls_table[current->syscall_num].rules = NULL;

        next = current->next;
        free(current);
        current = next;
    }

    syscalls_with_rules = NULL;
}

void reset_syscall_rules(){
    clean_syscall_rules();
}

bool check_rules(struct user_regs_struct regs){
    reg_t syscall_num = regs.orig_rax; // Syscall number is stored in orig_rax (x86_64)
    Syscall_argument* rules_for_syscall = get_rules_for_syscall_id(syscall_num);

    return (rules_for_syscall != NULL && rules_for_syscall[0].type != ___NONE_TYPE);
}

void record_syscall_with_rules(u_int32_t syscall_num) {
    Node_recorded_rules_t *new_node = (Node_recorded_rules_t*)malloc(sizeof(Node_recorded_rules_t));

    new_node->next = syscalls_with_rules;
    new_node->syscall_num = syscall_num;
    syscalls_with_rules = new_node;
}

// TODO: create custom handlers for special syscalls, that requires specific tracing strategy
//       for example with variadic or strange array arguments

// Will set rules for the syscall by provided name
// arguments must be in exact order (argument 0, argument 1, argument 2...)
void set_rules_for_syscall_name(char* name, char** arguments, u_int32_t arguments_length, Action_type action) {
    Syscall_argument* rules = NULL;
    u_int32_t i = 0;

    u_int32_t syscall_num = get_Syscall_hashmap(syscalls_map, name);
    if (syscall_num == HM_NOT_FOUND || 
        IS_EMPTY_SYSCALL(syscalls_table[syscall_num])
    ) return;

    rules = get_rules_for_syscall_id(syscall_num);

    if (rules != NULL) return; // rules for that syscall were already set

    record_syscall_with_rules(syscall_num);

    if (arguments_length == 0) {
        // rules are empty, no need to go further copying arguments
        syscalls_table[syscall_num].rules = &EMPTY_RULES;
        syscalls_table[syscall_num].action = action;
        printf("Setting empty rules for syscall %s\n", name);
        return;
    }

    // create new rules
    rules = (Syscall_argument*)malloc((arguments_length + 1) * sizeof(Syscall_argument));
    syscalls_table[syscall_num].rules = rules;
    syscalls_table[syscall_num].action = action;

    // TODO: when the argument type is ARRAY_TYPE, check it user specified a string or regex
    //       if yes: perform regex matching (if needed) and print it using "printf"
    //       if no: print all read bytes one by one using "putc"

    // TODO: when argument type is PAIR_TYPE: check forbidden values for both 

    // copying arguments, if no arguments were specified -> just allocate memory and mark, that this syscall is forbidden
    for (i = 0; i < arguments_length; i++) {
        char *endptr = NULL;
        printf("Setting up argument\n");

        rules[i].type = syscalls_table[syscall_num].arg_types[i];
        rules[arguments_length].type = ___NONE_TYPE; // must be none-terminated

        switch (rules[i].type) {
        case ADDRESS_TYPE:
            // assuming address starts with "0x" (16-base int), starting scanning number from second position
            if (check_regex(arguments[i], "^0x[0-9a-fA-F]+$") == 0) {
                rules[i].addr[0] = (uintptr_t)strtoull(arguments[i], &endptr, 16);
                rules[i].addr[1] = (uintptr_t)strtoull(arguments[i], &endptr, 16);
            } else if (check_regex(arguments[i], "^0x[0-9a-fA-F]+-0x[0-9a-fA-F]+$") == 0) {
                sscanf(arguments[i], "0x%llx-0x%llx", &rules[i].addr[0], &rules[i].addr[1]);
            } else {
                rules[i].addr[0] = 0;
                rules[i].addr[1] = 0;
                continue;
            }
            break;
        case UINT_64_TYPE:{
            u_int64_t value = ULLONG_MAX;
            strtoull_or_error(arguments[i], value, {
                continue;
            });
            rules[i].uint64 = value;
            break;
        }
        case UINT_32_TYPE: {
            u_int32_t value = ULONG_MAX;
            strtoul_or_error(arguments[i], value, {
                continue;
            });
            rules[i].uint32 = value;
            break;
        }
        case INT_32_TYPE: {
            int32_t value = LONG_MAX;
            strtol_or_error(arguments[i], value, {
                continue;
            });
            rules[i].int32 = value;
            break;
        }
        case INT_64_TYPE: {
            int64_t value = LLONG_MAX;
            strtoll_or_error(arguments[i], value, {
                continue;
            });
            rules[i].int64 = value;
            break;
        }
        case STRING_TYPE: {
            size_t len = strlen(arguments[i]);

            if (arguments[i][0] == 'r') {
                rules[i].is_regex = true;
                rules[i].str = (char*)malloc((len - 2) * sizeof(char));
                strncpy_with_esc(rules[i].str, arguments[i] + 2, len - 3);
            } else {
                rules[i].is_regex = false;
                rules[i].str = (char*)malloc((len - 1) * sizeof(char));
                // accounting for quotation marks (") at the start and end of string
                strncpy_with_esc(rules[i].str, arguments[i] + 1, len - 2);
            }

            break;
        }
        case ARRAY_TYPE: {
            // TODO: parse provided byte array
            break;
        }
        default: {
            rules[i].other = (void*)arguments[i];
            break;
        }
        }

    }

    Syscall_argument* r = get_rules_for_syscall_id(syscall_num);
    printf("Syscall %s, action: %d\n", name, action);
    for (i = 0; not(RULE_IS_NONE(r[i])); i++) {
        printf("Arg No %d ", i);
        switch (r[i].type) {
        case ADDRESS_TYPE:
            printf("type: ADDRESS_TYPE, val = %llx-%llx\n", r[i].addr[0], r[i].addr[1]);
            break;
        case UINT_64_TYPE:{
            printf("type: UINT_64_TYPE, val = %llu\n", r[i].uint64);
            break;
        }
        case INT_64_TYPE: {
            printf("type: INT_64_TYPE, val = %lld\n", r[i].int64);
            break;
        }
        case UINT_32_TYPE: {
            printf("type: UINT_32_TYPE, val = %u\n", r[i].uint32);
            break;
        }
        case INT_32_TYPE: {
            printf("type: INT_32_TYPE, val = %d\n", r[i].int32);
            break;
        }
        case STRING_TYPE: {
            printf("type: STR, val = %s\n", r[i].str);
            break;
        }
        case ARRAY_TYPE: {
            printf("type: ARR, val = %p\n", r[i].arr);
            break;
        }
        }
    }
}

bool cmp_syscall_argument(Syscall_argument argument, reg_t value, char* str_value) {
    if (argument.type == ___NONE_TYPE) return false;

    char *endptr = NULL;

    switch (argument.type) {
    case ADDRESS_TYPE:
        //printf("Comparing: %llx < %llx: %d", argument.addr, (u_int64_t)value, argument.addr < (u_int64_t)value);
        return (argument.addr[0] <= (u_int64_t)value && argument.addr[1] >= (u_int64_t)value);
    case UINT_64_TYPE:{
        printf("Comparing: %llu == %llu: %d", argument.uint64, (u_int64_t)value, argument.uint64 == (u_int64_t)value);
        return argument.uint64 == (u_int64_t)value;
    }
    case UINT_32_TYPE: {
        printf("Comparing: %u == %u: %d", argument.uint32, (u_int32_t)value, argument.uint32 == (u_int32_t)value);
        return argument.uint32 == (u_int32_t)value;
    }
    case INT_32_TYPE: {
        printf("Comparing: %d == %d: %d", argument.int32, (int32_t)value, argument.int32 == (int32_t)value);
        return argument.int32 == (int32_t)value;
    }
    case INT_64_TYPE: {
        printf("Comparing: %lld == %lld: %d", argument.int64, (int64_t)value, argument.int64 == (int64_t)value);
        return argument.int64 == (int64_t)value;
    }
    case STRING_TYPE: {
        printf("Comparing: %s == %s: %d", argument.str, str_value, strcmp(argument.str, str_value));
        if (argument.is_regex) {
            bool res = false;
            ssize_t len = strlen(str_value);
            // accunting for the quotes (") at the start and end of the matched string
            str_value[len - 1] = '\0';
            res = (check_regex(str_value + 1, argument.str) == 0);
            str_value[len - 1] = '"';
            return res;
        } else
            // accunting for the quotes (") at the start and end of the string
            return (strncmp(argument.str, str_value + 1, strlen(argument.str) - 1) == 0);

    }
    case ARRAY_TYPE: {
        // TODO: implement array comparison logic
        break;
    }
    default: {
        return argument.other == (void*)value;
    }
    }
}

byte_t* read_data_from_tracee(pid_t pid, reg_t addr, size_t length) {
    long word = 0;
    size_t sizeof_word = sizeof(word);
    size_t bytes_read = 0;
    byte_t* data = (byte_t*)malloc(length * sizeof(byte_t));
    size_t rem = length;
    size_t chunk_size = sizeof_word;

    errno = 0;
    while (bytes_read < length) {
        word = ptrace(PTRACE_PEEKDATA, pid, (void*)(addr + bytes_read), NULL);
        if (word == -1 && errno != 0) {
            // error
            return NULL;
        }

        rem = length - bytes_read;
        chunk_size = (rem >= sizeof_word)? sizeof_word : rem;

        memmove(data + bytes_read, &word, chunk_size);

        bytes_read += chunk_size;
    }

    return data;
}

char* read_str_from_tracee(pid_t pid, reg_t addr) {
    long word = 0;
    size_t sizeof_word = sizeof(word);
    size_t bytes_read = 0;
    const size_t default_len = 32;
    char* temp = NULL;
    char* data = (char*)malloc(default_len * sizeof(char));
    size_t capacity = default_len;
    size_t rem = default_len;
    size_t chunk_size = sizeof_word;

    errno = 0;
    while (true) {
        word = ptrace(PTRACE_PEEKDATA, pid, (void*)(addr + bytes_read), NULL);
        if (word == -1 && errno != 0) {
            // error
            free(data);
            return NULL;
        }

        if (bytes_read + chunk_size > capacity) {
            // data won't fit -> resize buffer
            capacity *= 2;
            temp = (char*)realloc(data, capacity * sizeof(char));
            if (temp == NULL) {
                // out of memory -> retun what was read
                free(data);
                return NULL;
            }
            data = temp;
        }

        memmove(data + bytes_read, &word, chunk_size);

        for (int i = 0; i < chunk_size; ++i) {
            if (data[bytes_read + i] == 0x0) // end of the string
                return data;
        }

        bytes_read += chunk_size;
    }
}

// TODO: process empty string in the rules and in outputs
Action_type print_blocked_syscall_arguments(reg_t syscall_num, pid_t pid, struct user_regs_struct regs, int trace_output_fd) {
    const Syscall_arg_type* types = get_syscall_argument_types(syscall_num);
    const reg_t raw_arguments[MAX_SYSCALL_ARGS_NUM] = {regs.rdi, regs.rsi, regs.rdx, regs.r10, regs.r8, regs.r9};
    char* str_arguments[MAX_SYSCALL_ARGS_NUM] = {};
    const u_int32_t DEFAULT_BUFER_LEN = 20;
    char* buffer_c = NULL;
    byte_t* buffer_b = NULL;
    const char* syscall_args_print_formats[] = {NULL, "%d", "%u", "%lld", "%llu", "\"%s\"", "0x%llx", "0x%llx", "0x%llx", "0x%llx"};
    Action_type required_action = syscalls_table[syscall_num].action; // entering this function automatically means, that syscall is assumed to be blocked

    u_int8_t buf_len = 0;
    u_int8_t argument_num = 0;
    u_int8_t i = 0;

    // collect arguments in form of array of Syscall_argument
    for (i = 0; types[i] != ___NONE_TYPE && i < MAX_SYSCALL_ARGS_NUM; i++) {
        if (types[i] == STRING_TYPE) {
            // read the string
            str_arguments[i] = read_str_from_tracee(pid, raw_arguments[i]);
            //sprintf(str_arguments[i], syscall_args_print_formats[types[i]], buffer_c);
        } else if (types[i] == ARRAY_TYPE) {
            // in regular syscall assuming that the very next argument is a length
            buffer_b = read_data_from_tracee(pid, raw_arguments[i], (size_t)raw_arguments[i + 1]);
            //str_arguments[i] = (char*)malloc((buf_len + 2 + 1) * sizeof(char));
            //sprintf(str_arguments[i], syscall_args_print_formats[types[i]], buffer);

        } else {
            // prepare argument for printing according to format
            buffer_c = (char*)malloc(99);
            buf_len = sprintf(buffer_c, syscall_args_print_formats[types[i]], raw_arguments[i]);
            str_arguments[i] = (char*)malloc((buf_len + 1) * sizeof(char));
            strcpy(str_arguments[i], buffer_c);
        }
    }
    argument_num = i;

    // check if syscall arguments actually are forbidden
    if (get_rules_for_syscall_id(syscall_num) != &EMPTY_RULES) {
        for (i = 0; i < argument_num && get_rules_for_syscall_id(syscall_num)[i].type != ___NONE_TYPE; i++) {
            Syscall_argument arg = get_rules_for_syscall_id(syscall_num)[i];
            if (not(cmp_syscall_argument(arg, raw_arguments[i], str_arguments[i]))) {
                required_action = NONE_ACTION;
                break;
            }

        }
    }

    switch (required_action) {
    case BLOCK:
        print_catched_syscall(trace_output_fd, "Blocked", get_name_for_syscall_id(syscall_num), argument_num, str_arguments[0], str_arguments[1], str_arguments[2], str_arguments[3], str_arguments[4], str_arguments[5]);
        break;
    case FILTER:
        print_catched_syscall(trace_output_fd, "Filtered", get_name_for_syscall_id(syscall_num), argument_num, str_arguments[0], str_arguments[1], str_arguments[2], str_arguments[3], str_arguments[4], str_arguments[5]);
        break;
    case NOTIFY:
        print_catched_syscall(trace_output_fd, "Detected", get_name_for_syscall_id(syscall_num), argument_num, str_arguments[0], str_arguments[1], str_arguments[2], str_arguments[3], str_arguments[4], str_arguments[5]);
        break;
    }


    for (int i = 0; i < argument_num; i++) {
        free(str_arguments[i]);
    }

    free(buffer_c);
    free(buffer_b);

    fflush(stdout);
    return required_action;
}
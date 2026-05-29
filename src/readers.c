#include "../include/readers.h"

Token_t* read_rules_from_file(Token_t filename) {
    Token_t* rules = NULL;
    u_int32_t rules_len = 0;
    FILE* rules_file = fopen(filename, "r");

    if (rules_file == NULL) {
        print_execution_error();
        return NULL;
    }

    u_int32_t arguments_number = 0;
    char* arguments[] = {NULL, NULL, NULL, NULL, NULL, NULL}; // 6 arguments to every rules entry
    char* line = NULL;
    size_t line_length = 0;
    const char* line_pattern = "^(deny|filter|notify):\\w+\\s*";
    const char* arg_pattern = "arg[0-5]=.+";
    ssize_t bytes_read = 0;
    Action_type action = NONE_ACTION;

    while (bytes_read != EOF) {
        bytes_read = getline(&line, &line_length, rules_file);
        if (bytes_read == EOF) {
            break;
        }
        line[bytes_read - 1] = '\0';
        if (strlen(line) < 2 || check_regex(line, line_pattern) != 0) continue;

        if (strncmp(line, "deny", 4) == 0) {
            action = BLOCK;
        } else if (strncmp(line, "filter", 6) == 0) {
            action = FILTER;
        } else if (strncmp(line, "notify", 6) == 0) {
            action = NOTIFY;
        }

        u_int32_t tokens_length = 0;
        Token_t* tokens = tokenize_with_strings(nth_token(line, ':', 1), &tokens_length, ' ');

        char* syscall_name = tokens[0];
        char** raw_arguments = tokens + 1;
        arguments_number = 0;

        if (raw_arguments != NULL) {
            // TODO: use strtol here to get argument's index and set arguments by them instead or relying on order
            for (u_int32_t i = 1; i < tokens_length; i++){
                check_regex(tokens[i], arg_pattern);
                char* endptr = NULL;
                u_int32_t index = strtoul(tokens[i] + 3, &endptr, 10);
                arguments[index] = nth_token(tokens[i], '=', 1);
                if (index + 1 > arguments_number)
                    arguments_number = index + 1;
            }
            fflush(stdout);
        } else {
            fflush(stdout);
        }

        set_rules_for_syscall_name(syscall_name, arguments, arguments_number, action);
        clean_tokens(tokens, tokens_length);
    }

    free(line);
    //if (line != NULL) free(line);

    fclose(rules_file);
}
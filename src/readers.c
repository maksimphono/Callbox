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

char* read_command_from_terminal() {
    char* raw_command = NULL;
    char* processed_raw_command = NULL;
    size_t command_length = 0;
    ssize_t bytes_read = getline(&raw_command, &command_length, stdin);
    if (bytes_read == -1) {
        free(raw_command);
        global_state.EXIT_GRACEFULLY = true;
        return NULL;
    }
    
    // if raw command starts with spaces and pipes or end with them
    if (check_regex(raw_command, "^(\\s*\\|+)|(\\|+\\s*)$") == 0) {
        goto invalid_syntax_error;
    }

    const char* pattern = "(\\|+\\s*\\|+)|(\\|+\\s*>+)|(>+\\s*\\|+)|(<+)"; // multiple pipes in row or spaces between pipes

    processed_raw_command = (char*)malloc((bytes_read + 1) * sizeof(char));

    char* src = raw_command;
    char* dest = processed_raw_command;
    bool state_reading_string = false;
    char* prev_str_end = raw_command;

    // remove spaces around '|' and '>'
    for (; *src != '\0' && *src != '\n'; src++) {
        if (*src == '\"') {
            if (state_reading_string) {
                prev_str_end = src + 1;
            } else {
                *src = '\0';
                if (check_regex(prev_str_end, pattern) == 0) {
                    goto invalid_syntax_error;
                }
                *src = '\"';
            }
            state_reading_string = not(state_reading_string);
            continue;
        } else
        if (not(state_reading_string)) {
            switch (*src) {
            case ' ': {
                if (
                    dest != processed_raw_command && 
                    *(dest - 1) != PSEUDO_SP && // spaces must not be consequent
                    *(dest - 1) != PSEUDO_PIPE && // space must not follow the pipe
                    *(dest - 1) != PSEUDO_ARR // spaces must not follow stdout redirection symbol
                ) {
                    *dest++ = PSEUDO_SP;
                }
                continue;
            }
            case '|': {
                if (
                    dest == processed_raw_command || 
                    *(dest - 1) == PSEUDO_SP
                ) {
                    *(--dest) = PSEUDO_PIPE; // replace previous character with current (pipe) and move to this prevoius character (pretend we haven't moved from it)
                }
                dest++;
                continue;
            }
            case '>': {
                *dest++ = PSEUDO_ARR;
                continue;
            }
            }
        }
        // default case: just copy the character
        *dest = *src;
        dest++;
        if (IS_ESC_SEQ(*src)) {
            // just skip the escape sequence
            *src++;
            *dest++ = *src;
        }
    }

    if (state_reading_string || check_regex(prev_str_end, pattern) == 0) {
        // unmatched quote (\") was detected (string wasn't closed)
        goto invalid_syntax_error;
    }

    size_t processed_length = (size_t)(dest - processed_raw_command);

    if (processed_raw_command[processed_length - 1] == ' ') {
        processed_length--;
    }

    processed_raw_command[processed_length] = '\0';

    free(raw_command);

    return processed_raw_command;

invalid_syntax_error:
    print_invalid_syntax();
    free(raw_command);
    free(processed_raw_command);
    return NULL;
}

#include "../include/readers.h"
#include "../include/print_message.h"

char esc_char(char ch) {
    switch(ch) {
    case 'a':  return '\a';
    case 'b':  return '\b';
    case 'e':  return '\e';
    case 'f':  return '\f';
    case 'n':  return '\n';
    case 'r':  return '\r';
    case 't':  return '\t';
    case 'v':  return '\v';
    case '\\': return '\\';
    case '\'': return '\'';
    case '\"': return '\"';
    case '\?': return '\?';
    case ' ':  return ' ';
    default:   return -1; // invalid escape sequence
    }
}

char* lex_string(FILE* file, char brk) {
    int capacity = 20;
    int len = 0;
    char* string = (char*)malloc(capacity * sizeof(char));
    char ch = getc(file);

    while (ch != EOF && ch != brk) {
        if (len >= capacity) {
            capacity *= 2;
            string = (char*)realloc_or_err(string, capacity + 1, {
                goto err;
            });
        }
        if (IS_ESC_SEQ(ch)) { // escape sequence
            ch = getc(file);
            ch = esc_char(ch);
            if (ch == -1) return NULL;
        }
        string[len] = ch;
        ++len;
        ch = getc(file);
    }

    if (ch == EOF) {
        // string wasn't closed
        goto err; 
    }

    string[len] = '\0';

    return string;

err:
    free(string);
    return NULL;
}

bool is_digit(char ch) {
    return ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f');
}

bool is_stopper(char ch) {
    // stopper is a character, that indicates, that parsing current token should
    // be stopped, like ',' or '\n'...
    switch (ch) {
    case ',':  return true;
    case ':':  return true;
    case '=':  return true;
    case '\n': return true;
    case ' ':  return true;
    case '\t': return true;
    case '{':  return true;
    case '}':  return true;
    default:   return false;
    }
}

Token next_token(FILE* file) {
    char ch = getc(file);

    while (ch != EOF && (ch == ' ' || ch == '\t')) {
        // skipping spaces and tabs
        ch = getc(file);
    }

    if (ch == EOF) return (Token){ TOK_EOF, NULL };

    if (ch == '"' || ch == '/') { // regex is marked with '/'
        // regex of string
        char* str = lex_string(file, ch);
        if (str == NULL) return (Token){TOK_ERR, NULL};
        return (ch == '"') ? (Token){ TOK_STRING, str } : (Token){ TOK_REGEX, str };
    }

    switch (ch) {
    case ':': return (Token){TOK_COLON,   NULL};
    case '{': return (Token){TOK_LBRACE,  NULL};
    case '}': return (Token){TOK_RBRACE,  NULL};
    case ',': return (Token){TOK_COMMA,   NULL};
    case '=': return (Token){TOK_EQ_SIGN, NULL};
    case '\n':return (Token){TOK_ENDL,    NULL};
    }

    // parse number or operator
    int capacity = 20;
    char* buffer = (char*)malloc(capacity * sizeof(char));
    bool parsing_number = true; // assuming that the token is a number at first
    int len = 0;
    while (ch != EOF && not(is_stopper(ch))) {
        if (is_digit(ch) || (len == 1 && ch == 'x')) { // it's a regular number of a hex number written as 0x123...
        } else {
            // not parsing a number, it's likely an operator
            parsing_number = false;
        }
        if (len >= capacity) {
            capacity *= 2;
            buffer = (char*)realloc_or_err(buffer, capacity + 1, {
                goto err;
            });
        }
        buffer[len] = ch;
        ++len;
        ch = getc(file);
    }

    buffer[len] = '\0'; // null-terminated
    if (is_stopper(ch)) 
        // it's a stopper - returning it back to the stream to read on the next call
        ungetc(ch, file);

    if (parsing_number) return (Token){TOK_NUMBER, buffer};
    return (Token){TOK_OPERATOR, buffer};

err:
    free(buffer); 
    return (Token){TOK_ERR, NULL};
}

/*
typedef struct Array_Node {
    byte_t value;
    Array_Node next;
} Array_Node;

Array_Node* add_node(Array_Node* node, byte_t value) {
    Array_Node* new = (Array_Node*)malloc(sizeof(Array_Node));
    new.value = value;
    new.next = node;
    return new;
}

void destroy_nodes(Array_Node* node) {
    Array_Node* next;
    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }
}
*/

typedef enum {
    ST_EXPECT_ACTION,
    ST_EXPECT_NAME,
    ST_EXPECT_ARGN,
    ST_EXPECT_VALUE,
} Parser_state;

byte_t* parse_array(FILE* file, size_t *length) {
    size_t capacity = 64;
    size_t len = 0;
    byte_t* array = (byte_t*)malloc(capacity * sizeof(byte_t));
    Token tok = next_token(file);

    while (tok.type != TOK_EOF && tok.type != TOK_RBRACE) {
        if (tok.type == TOK_ERR) {
            goto err;
        }
        if (tok.type == TOK_COMMA){
            tok = next_token(file);
            continue;
        }

        if (len >= capacity) {
            capacity *= 2;
            array = (byte_t*)realloc_or_err(array, capacity * sizeof(byte_t), {
                goto err;
            });
        }
        if (strncmp(tok.body, "0x", 2) == 0) // hex representation
            strtohex_or_error(tok.body + 2, array[len], { return NULL; });
        else
            array[len] = strtoul_or_err(tok.body, { return NULL; });
        
        free(tok.body);
        ++len;
        tok = next_token(file);
    }

    if (tok.type == TOK_EOF)
        // error: array wasn't closed
        goto err;

    if (len < capacity) {
        array = (byte_t*)realloc_or_err(array, len, {
            goto err;
        });
    }

// TODO: figure out why wrong pointer to length is passed
    *length = len;
    return array;

err:
    free(array);
    return NULL;
}

ExitStatus_t parse_rules_from_file(char* filename) {
    ExitStatus_t st = EXIT_UNKNOWN_ERR;
    FILE* file = fopen(filename, "r");
    Token tok = next_token(file);
    u_int32_t arguments_number = 0;
    Syscall_argument arguments[] = {EMPTY_RULES, EMPTY_RULES, EMPTY_RULES, EMPTY_RULES, EMPTY_RULES, EMPTY_RULES}; // 6 arguments to every rules entry
    Action_type action = NONE_ACTION;
    Parser_state state = ST_EXPECT_ACTION;
    char* syscall_name = NULL;
    u_int32_t index = 0;

    while (tok.type != TOK_EOF) {
        switch (tok.type) {
        case TOK_ERR:
            goto err;
        case TOK_COMMA:
            break;
        case TOK_ENDL: {
            // done parsing this syscall rules
            if (state == ST_EXPECT_ACTION && action == NONE_ACTION) {
                // the line is likely empty -> skip
                break;
            }
            if (action == NONE_ACTION || syscall_name == NULL) {
                // error: can't parse action or name
                goto err;
            }
            if (st = set_rules_for_syscall_name(syscall_name, arguments, arguments_number, action) != EXIT_SUCCESS_) {
                goto err;
            }

            free(syscall_name);
            action = NONE_ACTION;
            state = ST_EXPECT_ACTION;
            index = 0;
            arguments_number = 0;
            memset(arguments, 0x0, 6 * sizeof(Syscall_argument));
            break;
        }
        case TOK_OPERATOR: {
            if (state == ST_EXPECT_ACTION) {
                state = ST_EXPECT_NAME;

                if (strcmp(tok.body, "filter") == 0) {
                    action = FILTER;
                } else if (strcmp(tok.body, "deny") == 0) {
                    action = BLOCK;
                } else if (strcmp(tok.body, "notify") == 0) {
                    action = NOTIFY;
                } else {
                    // error: wrong action
                    goto err;
                }
                if (next_token(file).type != TOK_COLON) {
                    // error, missing sepqrator ':'
                    goto err;
                }
                free(tok.body);
            } else if (state == ST_EXPECT_ARGN && strncmp(tok.body, "arg", 3) == 0) {
                state = ST_EXPECT_VALUE;
                index = (u_int32_t)strtoul_or_err(tok.body + 3, {
                    // error: misformatted argument number 
                    goto err;
                });
                if (next_token(file).type != TOK_EQ_SIGN) {
                    // error: missing argument comparison operator ('=')
                    goto err;
                }
                if (index >= MAX_SYSCALL_ARGS_NUM) {
                    // error: index is too large
                    goto err;
                }
                if (arguments_number <= index) arguments_number = index + 1;
                free(tok.body);
            } else if (state == ST_EXPECT_NAME) {
                state = ST_EXPECT_ARGN;
                syscall_name = tok.body;
            } else {
                // error: wrong rule entry format
                goto err;
            }
            break;
        }
        case TOK_REGEX:
        case TOK_STRING: {
            if (state != ST_EXPECT_VALUE) {
                // error, not expecting value
                goto err;
            }
            state = ST_EXPECT_ARGN;

            arguments[index].type = STRING_TYPE;
            arguments[index].str = tok.body;
            arguments[index].is_regex = (tok.type == TOK_REGEX);
            break;
        }
        case TOK_NUMBER: {
            // TODO: parse negative numbers as well
            if (state != ST_EXPECT_VALUE) {
                // error, not expecting value
                goto err;
            }
            state = ST_EXPECT_ARGN;

            arguments[index].type = ULLONG_TYPE; // just uint64 by default, will be converted to the actual type later
            arguments[index].str = tok.body;
            break;
        }
        case TOK_LBRACE: {
            // reading array
            if (state != ST_EXPECT_VALUE) {
                // error, not expecting value
                goto err;
            }
            state = ST_EXPECT_ARGN;

            arguments[index].type = ARRAY_TYPE;
            byte_t* arr = parse_array(file, &arguments[index].arr_len);
            arguments[index].arr = arr;
            if (arr == NULL) goto err;
            break;
        }
        }
        tok = next_token(file);
    }

    return EXIT_SUCCESS_;
err:
    if (tok.body != NULL) free(tok.body);
    fclose(file);
    return st;
}
/*
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
*/
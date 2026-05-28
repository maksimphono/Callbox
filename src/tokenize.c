#include "../include/tokenize.h"

u_int32_t count_tokens(char* command, char separator) {
    if (command == NULL) return 0;
    u_int32_t tokens = 0;

    for (u_int32_t i = 0; i < strlen(command); i++) {
        if (command[i] == separator) tokens++;
    }

    return tokens + 1;
}

// Separates string into tokens (symbols in the string are not considered separators)
Token_t* tokenize_with_strings(char* command, u_int32_t* tokens_len, const char separator) {
    bool state_reading_string = false;
    bool state_reading_token = false;
    u_int32_t token_start = 0, token_end = 0;
    u_int32_t command_len = strlen(command);
    Token_t* tokens;

    *tokens_len = 0;
    // check that the quote matches another quote and no invalid escape sequence
    for (u_int32_t i = 0; i < command_len; i++) {
        if (command[i] == separator) {
            if (state_reading_string == false && state_reading_token == true) {
                // finishing to read a token
                (*tokens_len)++;
                state_reading_token = false;
            }
            continue;
        } else {
            if (state_reading_token == false)
                // starting to read a token
                state_reading_token = true;
            if (IS_ESC_SEQ(command[i])) {
                i++; // just skip this escape sequence
                continue;
            }
            if (command[i] == '\"') 
                state_reading_string = not(state_reading_string);
        }
    }
    if (state_reading_string) {
        // string was never closed
        goto invalid_syntax_error;
    }
    if (state_reading_token) (*tokens_len)++;
    tokens = (Token_t*)malloc((*tokens_len + 1) * sizeof(Token_t));
    u_int32_t tokens_num = 0;

    state_reading_token = false;
    state_reading_string = false;
    u_int32_t i = 0;
    for (i = 0; i < command_len; i++) {
        if (command[i] == separator) {
            if (state_reading_string == false && state_reading_token == true) {
                // finishing to read a token
                token_end = i;
                
                tokens[tokens_num] = (Token_t)malloc((token_end - token_start + 1) * sizeof(char));
                strncpy(tokens[tokens_num], command + token_start, token_end - token_start);
                tokens[tokens_num][token_end - token_start] = '\0';
                tokens_num++;
                token_end = token_start = 0;
                state_reading_token = false;
            }
            continue;
        } else {
            if (state_reading_token == false)
                // starting to read a token
                token_start = i;
            // string encountered
            state_reading_token = true;
            if (IS_ESC_SEQ(command[i])) {
                i++; // just skip this escape sequence
                continue;
            }
            if (command[i] == '\"') 
                state_reading_string = not(state_reading_string);
        }
    }

    if (state_reading_token) {
        token_end = i;
        tokens[tokens_num] = (Token_t)malloc((token_end - token_start + 1) * sizeof(char));
        strncpy(tokens[tokens_num], command + token_start, token_end - token_start);
        tokens[tokens_num][token_end - token_start] = '\0';
    }
    tokens[*tokens_len] = NULL; // last token must be empty

    return tokens;

invalid_syntax_error:
    return NULL;
}

// Will return list of tokens after separating the string by the separator character
// Preserves initial string in it's original state
Token_t* tokenize_limit(char* command, u_int32_t* tokens_len, const char separator, u_int32_t limit) {
    // returns tokens of the command (all words, separated by space)
    if (strlen(command) == 0) return NULL;
    u_int32_t tokens_length = min(count_tokens(command, separator), limit);
    Token_t* tokens = (Token_t*)malloc((tokens_length + 1) * sizeof(Token_t));
    u_int32_t command_len = strlen(command);
    const char sep[] = {separator, '\0'};
    u_int32_t id = 0;
    u_int32_t* zero_ids = (u_int32_t*)malloc((tokens_length + 1) * sizeof(u_int32_t));

    Token_t token = strtok(command, sep);
    zero_ids[id++] = strlen(token);
    u_int32_t i = 0;

    while (token != NULL && i < limit) {
        tokens[i] = (Token_t)malloc((strlen(token) + 1) * sizeof(char));
        strcpy(tokens[i], token);

        token = strtok(NULL, sep);
        if (token != NULL) {
            zero_ids[id] = zero_ids[id - 1] + strlen(token) + 1;
            id++;
        }
        i++;
    }

    // return separators on their respective places
    for (int i = 0; i < id && zero_ids[i] < command_len; i++) {
        command[zero_ids[i]] = separator;
    }
    *tokens_len = tokens_length;
    tokens[tokens_length] = NULL;
    free(zero_ids);

    return tokens;

invalid_syntax_error:
    clean_tokens(tokens, i + 1);
    free(zero_ids);
    return NULL;
}

Token_t* tokenize(char* command, u_int32_t* tokens_len, const char separator) {
    return tokenize_limit(command, tokens_len, separator, __UINT32_MAX__);
}


Token_t nth_token(char* string, char separator, u_int32_t n) {
    u_int32_t i = 0;
    const char sep[] = {separator, '\0'};
    Token_t token = strtok(string, sep);

    while (token != NULL && i < n) {
        token = strtok(NULL, sep);
        i++;
    }

    if (i == n) return token;

    return NULL;
}

void clean_tokens(Token_t* tokens, u_int32_t tokens_length) {
    if (tokens == NULL) return;

    for (u_int32_t i = 0; i < tokens_length; i++) {
        if (tokens[i] != NULL) {
            
            free(tokens[i]);
        }
    }
    free(tokens);
}

#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PATH_MAX 200

#define IS_ESC_SEQ(s) (s == '\\')

#define min(a, b) ((a < b)?a:b)
#define not(v) ((v) == false)

typedef char* Token_t;

u_int32_t count_tokens(char* command, char separator);

// Note, that each time tokens are created using these functions, 
// they must be cleaned with clean_tokens() function
Token_t* tokenize_with_strings(char* command, u_int32_t* tokens_len, const char separator);

// Will return list of tokens after separating the string by the separator character
// Preserves initial string in it's original state
Token_t* tokenize_limit(char* command, u_int32_t* tokens_len, const char separator, u_int32_t limit);

Token_t* tokenize(char* command, u_int32_t* tokens_len, const char separator);

Token_t nth_token(char* string, char separator, u_int32_t n);

void clean_tokens(Token_t* tokens, u_int32_t tokens_length);

#endif

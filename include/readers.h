#ifndef _READERS_H_
#define _READERS_H_

#include "defs.h"
#include "tokenize.h"
#include "sandbox.h"
#include "utils.h"

typedef enum {
    TOK_COMMA,      // ,
    TOK_COLON,      // :
    TOK_LBRACE,     // {
    TOK_RBRACE,     // }
    TOK_EQ_SIGN,    // =
    TOK_OPERATOR,   // argX OR deny OR notify...
    TOK_NUMBER,     // 123
    TOK_STRING,     // string
    TOK_REGEX,      // regular expression
    TOK_ENDL,       // \n
    TOK_EOF,        // EOF
    TOK_ERR         // ERROR
} Type;

typedef struct Token {
    Type type;
    char* body;
} Token;

Token next_token(FILE* file);

Token_t* read_rules_from_file(Token_t filename);

#endif
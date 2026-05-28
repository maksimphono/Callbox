#ifndef _BUILDINS_H_
#define _BUILDINS_H_

#include <unistd.h>

#include "defs.h"
#include "tokenize.h"
#include "utils.h"
#include "readers.h"

bool is_buildin(char* cmd);

void handle_buildin_cd(Token_t* tokens, u_int32_t tokens_leng);

void handle_buildin_env(Token_t* tokens,  u_int32_t tokens_leng);

void handle_buildin_export(Token_t* tokens, u_int32_t tokens_leng);

void handle_buildin_exit(Token_t* tokens, u_int32_t tokens_leng);

void handle_buildin_sandbox(Token_t* tokens, u_int32_t tokens_leng);

Handler_fn find_buildin_handler(Token_t cmd);

extern Cmd_Handler_map_entry_t cmd_handler_map[];

extern const size_t cmd_handler_map_size;

Handler_fn find_buildin_handler(Token_t cmd);

#endif
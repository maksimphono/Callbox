#include "../include/defs.h"
#include "../include/executors.h"
#include "../include/print_message.h"
#include "../include/readers.h"
#include "../include/sandbox.h"
#include "../include/tokenize.h"
#include "../include/utils.h"
#include "../include/hashmap.h"

Token_t* prepare_tokens(int argc, char** argv){
    Token_t* tokens = (Token_t*)malloc((argc) * sizeof(Token_t));
    memcpy(tokens, argv + 1, (argc - 1) * sizeof(char*));
    tokens[argc - 1] = NULL;
    return tokens;
}


int main(int argc, char** argv) {
    init_syscall_rules();

    Token_t* tokens = prepare_tokens(argc, argv);

    execute_commands_workflow(tokens, (u_int32_t)argc - 1);

    free(tokens);

    del_syscall_rules();

    return 0;
}

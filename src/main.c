#include "../include/defs.h"
#include "../include/executors.h"
#include "../include/print_message.h"
#include "../include/readers.h"
#include "../include/sandbox.h"
#include "../include/tokenize.h"
#include "../include/utils.h"
#include "../include/hashmap.h"


int main(int argc, char** argv) {
    init_syscall_rules();

    Token_t* tokens = (Token_t*)malloc((argc) * sizeof(Token_t));
    memcpy(tokens, argv + 1, (argc - 1) * sizeof(char*));
    tokens[argc - 1] = NULL;

    u_int32_t tokens_length = (u_int32_t)argc - 1;

    execute_commands_workflow(tokens, tokens_length);

    free(tokens);

    del_syscall_rules();

    return 0;
}

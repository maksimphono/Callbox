#include "../include/defs.h"
#include "../include/executors.h"
#include "../include/print_message.h"
#include "../include/readers.h"
#include "../include/sandbox.h"
#include "../include/tokenize.h"
#include "../include/utils.h"
#include "../include/hashmap.h"
#include "../include/cli_arguments.h"

Token_t* prepare_tokens(int argc, const char** argv){
    Token_t* tokens = (Token_t*)malloc((argc) * sizeof(Token_t));
    memcpy(tokens, argv + 1, (argc - 1) * sizeof(char*));
    tokens[argc - 1] = NULL;
    return tokens;
}


int main(u_int32_t argc, const char** argv) {
    u_int32_t arg_num = 0;
    Arguments* cli_arguments = scan_cli_arguments(&arg_num, argc, argv);

    if (cli_arguments == NULL) {
        return 1;
    }

    init_syscall_rules();

    Token_t* tokens = prepare_tokens(argc - arg_num, argv + arg_num);

    execute_commands_workflow(tokens, (u_int32_t)(argc - arg_num) - 1, cli_arguments);

    free(tokens);

    del_syscall_rules();

    return 0;
}

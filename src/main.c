#include "../include/defs.h"
#include "../include/buildins.h"
#include "../include/executors.h"
#include "../include/print_message.h"
#include "../include/readers.h"
#include "../include/sandbox.h"
#include "../include/tokenize.h"
#include "../include/utils.h"
#include "../include/hashmap.h"

global_state_t global_state;

void cleanup(char* raw_command){
    global_state.mode = NORMAL;
    if (raw_command != NULL)
        free(raw_command);
}

int main(int argc, char** argv) {
    init_syscall_rules();

    Token_t* tokens = (Token_t*)malloc((argc) * sizeof(Token_t));
    memcpy(tokens, argv + 1, (argc - 1) * sizeof(char*));
    tokens[argc - 1] = NULL;

    u_int32_t n = (u_int32_t)argc;

    execute_commands_workflow(tokens, argc - 1);

    del_syscall_rules();

    return 0;
}

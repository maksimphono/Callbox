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

int main(int arg_n, char** arg_vc) {
    char* raw_command = NULL;
    CommandExecutionResult_t result = DEFAULT_EXECUTION_RESULT;

    init_environment();

    while(1) {
        print_prompt();
        raw_command = read_command_from_terminal();

        if (raw_command != NULL){
            //execute_single_command(raw_command, &result);
            execute_commands_workflow(raw_command, &result);
        }

        if (result.global_error) {
            print_execution_error();
            cleanup(raw_command);
            clean_syscall_rules();
            exit(1);
        } else if (global_state.EXIT_GRACEFULLY) {
            //clean_syscall_rules();
            del_syscall_rules();
            cleanup(raw_command);
            exit(0);
        }

        cleanup(raw_command);
        reset_syscall_rules();
        global_state.mode = NORMAL;
    }
    del_syscall_rules();

    return 0;
}

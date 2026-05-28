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
    init_environment();

    

    del_syscall_rules();

    return 0;
}

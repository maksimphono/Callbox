#include "buildins.h"

bool is_buildin(char* cmd){
    return find_buildin_handler(cmd) != NULL;
}

void handle_buildin_cd(Token_t* tokens, u_int32_t tokens_leng) {
    if (tokens_leng != 2) {
        print_invalid_syntax();
        return;
    }
    char* path_with_home = NULL;
    char* path = tokens[1];

    if (path[0] == '~') {
        // home directory
        char* home_dir = getenv("HOME");
        path_with_home = (char*)malloc((strlen(home_dir) + strlen(path)) * sizeof(char));
        strcpy(path_with_home, home_dir);
        strcpy(path_with_home + strlen(home_dir), path + 1);
        path = path_with_home;
    }

    if (chdir(path) == -1) {
        print_execution_error();
    }
    if (path_with_home != NULL) free(path_with_home);

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (setenv("PWD", cwd, 1) != 0) {
            print_execution_error();
        }
    } else {
        print_execution_error();
    }
}

void handle_buildin_env(Token_t* tokens,  u_int32_t tokens_leng) {
    
    for (char** env_var = __environ; *env_var != NULL; env_var++) {
        puts(*env_var);
    }
}

void handle_buildin_export(Token_t* tokens, u_int32_t tokens_leng) {
    if (tokens_leng < 2) {
        print_invalid_syntax();
        return;
    }

    char* name, *value = NULL;
    const char* pattern = "\\w+=[^=]+";

    for (u_int32_t i = 1; i < tokens_leng; i++) {
        if (check_regex(tokens[i], pattern) == 0) {
            name = strtok(tokens[i], "=");
            value = strtok(NULL, "=");

            setenv(name, value, 1);
        }
    }
}

void handle_buildin_exit(Token_t* tokens, u_int32_t tokens_leng) {
    global_state.EXIT_GRACEFULLY = true;
}

void handle_buildin_sandbox(Token_t* tokens, u_int32_t tokens_leng) {
    if (tokens_leng < 2) {
        print_invalid_syntax();
    }
    read_rules_from_file(tokens[1]);

    global_state.mode = SANDBOX;
}

Cmd_Handler_map_entry_t cmd_handler_map[] = {
    {"cd", handle_buildin_cd},
    {"export", handle_buildin_export},
    {"exit", handle_buildin_exit},
    {KEYWORD_SANDBOX, handle_buildin_sandbox}
    //{"env", handle_buildin_env}
};

const size_t cmd_handler_map_size = sizeof(cmd_handler_map) / sizeof(Cmd_Handler_map_entry_t);

Handler_fn find_buildin_handler(Token_t cmd){
    for (u_int32_t i = 0; i < cmd_handler_map_size; i++) {
        if (strcmp(cmd, cmd_handler_map[i].cmd) == 0) {
            return cmd_handler_map[i].handler;
        }
    }

    return NULL;
}

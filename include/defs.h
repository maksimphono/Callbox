#ifndef _DEFS_H_
#define _DEFS_H_

#include <stdbool.h>
#include <sys/types.h>

#define VERSION "1.0.1"

typedef unsigned long long reg_t; // data type to store value, read from 64-bit registers

#define PATH_MAX 200

#define SYSCALL_ARG_TYPES_LENGTH 8

#define MAX_SYSCALL_ARGS_NUM 6

#define DEFAULT_ENV_PATH "/bin"
#define DEFAULT_ENV_LANG "en_US.UTF-8"
#define DEFAULT_ENV_VERSION VERSION

#define IS_SINGLE_COMMAND true
#define IS_NOT_SINGLE_COMMAND false

#define IS_CHILD(p_id) (p_id == 0)

#define IS_ESC_SEQ(s) (s == '\\')

#define min(a, b) ((a < b)?a:b)
#define not(v) ((v) == false)

#define PSEUDO_SP (char)(0x13) // used to replace spaces (' ') in the string, allowing strings to be passed through tokenization without being separated by spaces
#define PSEUDO_PIPE (char)(0x14)
#define PSEUDO_ARR (char)(0x15)

#define IS_STDOUT_REDIRECTION(token) (token[0] == PSEUDO_ARR)
#define IS_NOT_STDOUT_REDIRECTION(token) (token[0] != PSEUDO_ARR)

#define KEYWORD_SANDBOX "sandbox"

typedef enum {NORMAL, SANDBOX} Mode;

typedef struct {
    char* current_path;
    bool EXIT_GRACEFULLY;
    Mode mode;
} global_state_t;

extern global_state_t global_state;

typedef void (*Handler_fn)(char**, u_int32_t);
typedef struct {
    const char* cmd;
    Handler_fn handler;
} Cmd_Handler_map_entry_t;

#define DEFAULT_EXECUTION_RESULT { \
    false, \
    0 \
}

// Hashmap:
#define HM_NOT_FOUND (u_int32_t)-1



#endif
#ifndef _EXECUTORS_H_
#define _EXECUTORS_H_

#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "defs.h"
#include "tokenize.h"
#include "sandbox.h"
#include "utils.h"
#include "buildins.h"

typedef struct {
    bool global_error;
    int exit_code;
} CommandExecutionResult_t;

void execute_buildin_command(Token_t* tokens, u_int32_t tokens_length, CommandExecutionResult_t* execution_result);

Trace_result run_tracer(pid_t pid, pid_t group_id);
// Executes custom (non-buildin) command from executable file
void execute_custom_command(Token_t* tokens, u_int32_t tokens_length, CommandExecutionResult_t* execution_result, int* pid, int* prev_pipe_read_fd, bool is_last, bool is_single, pid_t group_id);

void trace_custom_command(Token_t* tokens, u_int32_t tokens_length, CommandExecutionResult_t* execution_result, int* pid, int* prev_pipe_read_fd, int pipefd[2], bool is_last, bool is_single, pid_t group_id);

CommandExecutionResult_t* execute_single_command(char* command, CommandExecutionResult_t* execution_result);

pid_t start_dummy_leader();

CommandExecutionResult_t* execute_commands_workflow(char* _raw_command, CommandExecutionResult_t* result);

#endif
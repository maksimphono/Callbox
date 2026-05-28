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

Trace_result run_tracer(pid_t pid, pid_t group_id);
// Executes custom (non-buildin) command from executable file

void trace_custom_command(Token_t* tokens, u_int32_t tokens_length, CommandExecutionResult_t* execution_result, int* pid, int* prev_pipe_read_fd, int pipefd[2], bool is_last, bool is_single, pid_t group_id);

pid_t start_dummy_leader();

CommandExecutionResult_t* execute_commands_workflow(Token_t*, u_int32_t);

#endif
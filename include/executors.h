#ifndef _EXECUTORS_H_
#define _EXECUTORS_H_

#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "defs.h"
#include "tokenize.h"
#include "sandbox.h"
#include "readers.h"
#include "utils.h"
#include "cli_arguments.h"

typedef struct {
    bool global_error;
    int exit_code;
} CommandExecutionResult_t;

#define ptrace_or_error(cmd, a, b, mode) if (ptrace(cmd, a, b, mode) == -1) { return UNKNOWN_ERROR; }

Trace_result run_tracer(pid_t pid, pid_t group_id, int);
// Executes custom (non-buildin) command from executable file

int trace_program(Token_t* tokens, u_int32_t tokens_length, CommandExecutionResult_t* execution_result, int, int);

CommandExecutionResult_t* execute_commands_workflow(Token_t*, u_int32_t, Arguments* cli_arguments);

#endif
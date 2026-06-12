#include "../include/executors.h"

Trace_result run_tracer(pid_t pid, pid_t group_id, int trace_output_fd) {
    int status;
    bool in_syscall = false;
    bool is_filtered = false;

    // Wait for the child to stop (first stop is right after PTRACE_TRACEME)
    if (waitpid(pid, &status, 0) == -1) {
        return UNKNOWN_ERROR;
    }

    ptrace_or_error(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD);

    while (WIFSTOPPED(status) || WIFEXITED(status)) {
        struct user_regs_struct regs;

        ptrace_or_error(PTRACE_GETREGS, pid, 0, &regs);

        if (WIFSTOPPED(status) && (WSTOPSIG(status) & 0x80)) {
            if (not(in_syscall)) {
                reg_t syscall_num = regs.orig_rax; // Syscall number is stored in orig_rax (x86_64)

                if (get_rules_for_syscall_id(syscall_num) != NULL){
                    // syscall propably should be blocked
                    Action_type required_action = print_blocked_syscall_arguments(syscall_num, pid, regs, trace_output_fd);

                    if (required_action == BLOCK){
                        // syscall really should be blocked, exiting
                        if (group_id > 0) {
                            if (kill(-group_id, SIGKILL) == -1) { // killing all other processes in the group instantly
                                // Fallback to exit_group on the tracee if group kill fails (unlikely)
                                regs.orig_rax = 231; regs.rdi = 2;
                                ptrace(PTRACE_SETREGS, pid, 0, &regs);
                            }
                            ptrace_or_error(PTRACE_CONT, pid, 0, 0);

                            return BLOCKED_SYSCALL;
                        }
                        regs.orig_rax = 231;
                        regs.rdi = 1;

                        ptrace_or_error(PTRACE_SETREGS, pid, 0, &regs);

                        ptrace_or_error(PTRACE_CONT, pid, 0, 0);

                        return BLOCKED_SYSCALL;
                    } else if (required_action == FILTER) {
                        // first stage of filtering: on syscall enter change it's number, make kernel skip it's execution
                        regs.orig_rax = -1;
                        ptrace_or_error(PTRACE_SETREGS, pid, 0, &regs);

                        is_filtered = true;
                    }
                }
                in_syscall = true;
            }
            else {
                long return_value = regs.rax;
                in_syscall = false;

                if (is_filtered) {
                    // second stage of filtering: on syscall exit make it look like syscall execution returns access error
                    regs.rax = -EACCES;
                    ptrace_or_error(PTRACE_SETREGS, pid, 0, &regs);

                    is_filtered = false;
                }
            }
        // Resume execution and wait for the next syscall stop
        }
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) {
            break; 
        }
        // Wait for the tracee to stop again (either due to syscall or termination)

        if (waitpid(pid, &status, 0) == -1) {
            return UNKNOWN_ERROR;
        }

        // If the child terminated, exit the tracing loop
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            break;
        }
    }
    return SUCCESS;
}


int trace_program(Token_t* tokens, u_int32_t tokens_length, CommandExecutionResult_t* execution_result, int prev_pipe_read_fd, int trace_output_fd) {
    pid_t tracee_pid;

    tracee_pid = fork();

    if (tracee_pid == -1) {
        print_execution_error();
        exit(1);
    }

    if (IS_CHILD(tracee_pid)) { // TRACEE
        setpgid(0, 0);

        if (prev_pipe_read_fd != STDIN_FILENO) {
            if (dup2(prev_pipe_read_fd, STDIN_FILENO) == -1) {
                _exit(UNKNOWN_ERROR);
            }
            close(prev_pipe_read_fd); 
        }
            
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1) {
            _exit(UNKNOWN_ERROR);
        }
        kill(getpid(), SIGSTOP);

        if (execvp(tokens[0], tokens) == -1) {
            /*
            When execvp() fails I must check if it was due to non-existing command,
            if it was -> I'm exiting with 127 (command not found), 
            otherwise just exiting with general error
            Then let the PARENT handle the error, depending on the exit code
            */
            if (errno == ENOENT) {
                // desired command wasn't found
                _exit(127);
            } else {
                // general error
                _exit(UNKNOWN_ERROR);
            }
        }
        _exit(SUCCESS);
    } else {

        // since the child now owns the necessary FD 0 copy).
        if (prev_pipe_read_fd != STDIN_FILENO) {
            close(prev_pipe_read_fd);
        }

        Trace_result tracer_result = run_tracer(tracee_pid, tracee_pid, trace_output_fd);
        int tracee_status;

        waitpid(tracee_pid, &tracee_status, 0);

        if (tracer_result == SUCCESS || tracer_result == BLOCKED_SYSCALL) {
            return (SUCCESS);
        }
        if (tracer_result != SUCCESS && WIFEXITED(tracee_status)) {
            // Tracing ended, and Tracee exited normally (status 0-255) -> just exit with this status
            return (WEXITSTATUS(tracee_status));
        } else if (WIFSIGNALED(tracee_status)) {
            // tracee was terminated (for example by syscall block)
            int term_sig = WTERMSIG(tracee_status);
            if (tracer_result == SUCCESS) {
                return (SUCCESS);
            } else {
                return (128 + term_sig); 
            }
        } else {
            // Fallback for unexpected status (e.g., still stopped).
            return (UNKNOWN_ERROR); 
        }
        return (tracer_result);
    }

    return UNKNOWN_ERROR;
}


// Executes full pipeline of commands (series of commands, connected by pipes '|')
ExitStatus_t execute_commands_workflow(Token_t* tokens, u_int32_t tokens_length, Arguments* cli_arguments) {
    // it's a pipeline of commands
    ExitStatus_t exit_status = EXIT_UNKNOWN_ERR;
    int prev_pipe_read_fd = STDIN_FILENO; // read from stdin by default
    int trace_output_fd = STDOUT_FILENO;

    if (cli_arguments->input_file != NULL)
        prev_pipe_read_fd = open(cli_arguments->input_file, O_RDONLY);

    if (cli_arguments->rules_file != NULL)
        if (exit_status = parse_rules_from_file(cli_arguments->rules_file) != 0) {
            if (trace_output_fd != STDOUT_FILENO) 
                close(trace_output_fd);
            printf("Error with setting rules\n");
            return exit_status;
        }
    else {
        print_empty_rules();
    }

    if (cli_arguments->trace_output_file != NULL) {
        trace_output_fd = open(cli_arguments->trace_output_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    }

    int trace_result = trace_program(
        tokens, tokens_length, NULL, prev_pipe_read_fd, trace_output_fd
    );

    if (trace_output_fd != STDOUT_FILENO) 
        close(trace_output_fd);

    //if (group_id > 0) {
    //    int status;
    //    // Use SIGKILL to terminate the sleep process quickly
    //    if (kill(group_id, SIGKILL) == -1 && errno != ESRCH) {
    //    }
    //    waitpid(group_id, &status, 0); 
    //}

    return EXIT_SUCCESS_;
}

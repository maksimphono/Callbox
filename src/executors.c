#include "../include/executors.h"

// TODO: refactor this
Trace_result run_tracer(pid_t pid, pid_t group_id) {
    int status;
    bool in_syscall = false;

    // Wait for the child to stop (first stop is right after PTRACE_TRACEME)
    if (waitpid(pid, &status, 0) == -1) {
        return UNKNOWN_ERROR;
    }

        if (ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD) == -1) {
        return UNKNOWN_ERROR;
    }

    while (WIFSTOPPED(status)) {
        struct user_regs_struct regs;

        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
            return UNKNOWN_ERROR;
        }

        if (WSTOPSIG(status) & 0x80 || not(in_syscall)) {
            reg_t syscall_num = regs.orig_rax; // Syscall number is stored in orig_rax (x86_64)

            if (get_rules_for_syscall_id(syscall_num) != NULL){
                // syscall propably should be blocked
                Action_type required_action = print_blocked_syscall_arguments(syscall_num, pid, regs);

                if (required_action == BLOCK){
                    // syscall really should be blocked, exiting
                    if (group_id > 0) {
                        if (kill(-group_id, SIGKILL) == -1) { // killing all other processes in the group instantly
                            // Fallback to exit_group on the tracee if group kill fails (unlikely)
                            regs.orig_rax = 231; regs.rdi = 2;
                            ptrace(PTRACE_SETREGS, pid, 0, &regs);
                            ptrace(PTRACE_CONT, pid, 0, 0);
                        }

                        return BLOCKED_SYSCALL;
                    }
                    regs.orig_rax = 231;
                    regs.rdi = 1;

                    if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1) {
                        return UNKNOWN_ERROR;
                    }

                    if (ptrace(PTRACE_CONT, pid, 0, 0) == -1) {
                        return 1;
                    }

                    return BLOCKED_SYSCALL;
                } else if (required_action == FILTER) {
                    // to indicate that syscall must not be executed (kernel must jump to syscall exit immediately)
                    regs.rax = -1;
                    regs.orig_rax = -1;
                    if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1) {
                        return UNKNOWN_ERROR;
                    }

                    if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) {
                        return UNKNOWN_ERROR;
                    }

                    return FILTERED_SYSCALL;
                } else if (required_action == NOTIFY) {
                    if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1) {
                        return UNKNOWN_ERROR;
                    }
                    if (ptrace(PTRACE_CONT, pid, 0, 0) == -1) {
                        return UNKNOWN_ERROR;
                    }
                    return NOTIFIED_SYSCALL;
                }
            }
            in_syscall = true; 
        }
        else {
            long return_value = regs.rax;
            in_syscall = false; 
        }
        // Resume execution and wait for the next syscall stop
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


void trace_custom_command(Token_t* tokens, u_int32_t tokens_length, CommandExecutionResult_t* execution_result, int* pid, int* prev_pipe_read_fd, int pipefd[2], bool is_last, bool is_single, pid_t group_id) {
    *pid = fork(); // creating Tracer process, now Main process will wait for it to finish execution

    if (*pid == -1) {
        return;
    }

    if (IS_CHILD(*pid)) { // TRACER

        pid_t tracee_pid;

        if (group_id > 0){
            if (setpgid(0, group_id) == -1) { // add tracer to the process group
                _exit(1);
            }
        }
            
        if (is_single) {
            pipefd[0] = STDIN_FILENO;
            pipefd[1] = STDOUT_FILENO;
        }

        tracee_pid = fork();

        if (tracee_pid == -1) {
            print_execution_error();
            // Clean up resources and exit loop
            exit(1);
        }

        if (IS_CHILD(tracee_pid)) { // TRACEE
            if (group_id > 0) {
                if (setpgid(0, group_id) == -1) { // add child process to the process group
                       _exit(1);
                }
            }

            if (*prev_pipe_read_fd != STDIN_FILENO) {
                // Redirect    //  stdin (FD 0) to the read end of the previous pipe
                if (dup2(*prev_pipe_read_fd, STDIN_FILENO) == -1) {
                    _exit(1);
                }
                // Child closes its inherited copy of the previous pipe's read end
                close(*prev_pipe_read_fd); 
            }

            // If the output needs to go to a pipe
            if (not(is_single) && is_last) {
                // Redirect stdout (FD 1) to the write end of the current pipe
                if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                    _exit(1);
                }
                
                close(pipefd[1]); // Close write end (copied to FD 1)
                close(pipefd[0]); // Close read end (not needed by writer)
            }

            u_int32_t stdout_targets_num;
            int* stdout_targets = find_stdout_redirect_targets(tokens, tokens_length, &stdout_targets_num);
            if (stdout_targets_num > 0 && dup2(stdout_targets[stdout_targets_num - 1], STDOUT_FILENO) == -1) {
                _exit(1);
            }
            for (u_int32_t i = 0; i < stdout_targets_num; i++) {
                close(stdout_targets[i]);
            }

            if (is_buildin(tokens[0])) {
                _exit(0); // don't do anything, exit immediately
            } else {
                if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1) {
                    _exit(1);
                }
                kill(getpid(), SIGSTOP);

                if (execvp_ignore_redirect(tokens[0], tokens, tokens_length) == -1) {
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
                        _exit(1);
                    }
                }
            }
        } else {

            // since the child now owns the necessary FD 0 copy).
            if (*prev_pipe_read_fd != STDIN_FILENO) {
                close(*prev_pipe_read_fd);
            }

            if (not(is_single) && is_last) {
                close(pipefd[1]); 
                close(pipefd[0]);
            }

            Trace_result tracer_result = run_tracer(tracee_pid, group_id);
            int tracee_status;

            waitpid(tracee_pid, &tracee_status, 0);

            if (tracer_result == SUCCESS) {
                _exit(SUCCESS);
            }
            else if (tracer_result == BLOCKED_SYSCALL) {
                _exit(SUCCESS);
            }
            else if (tracer_result == NOTIFIED_SYSCALL) {
                _exit(SUCCESS);
            }
            else if (tracer_result == FILTERED_SYSCALL) {
                // must get all the registers again to reset them
                struct user_regs_struct regs;

                if (ptrace(PTRACE_GETREGS, tracee_pid, 0, &regs) == -1) {
                    _exit(UNKNOWN_ERROR);
                }
                // notify about access error in kernel (just to have something to return)
                regs.rax = -EACCES;
                if (ptrace(PTRACE_SETREGS, tracee_pid, 0, &regs) == -1) {
                    _exit(UNKNOWN_ERROR);
                }
                if (ptrace(PTRACE_CONT, tracee_pid, 0, 0) == -1) {
                    _exit(UNKNOWN_ERROR);
                }

                _exit(SUCCESS);
            }
            if (tracer_result != SUCCESS && WIFEXITED(tracee_status)) {
                // Tracing ended, and Tracee exited normally (status 0-255) -> just exit with this status
                _exit(WEXITSTATUS(tracee_status));
            } else if (WIFSIGNALED(tracee_status)) {
                // tracee was terminated (for example by syscall block)
                int term_sig = WTERMSIG(tracee_status);
                if (tracer_result == SUCCESS) {
                    _exit(SUCCESS);
                } else {
                    _exit(128 + term_sig); 
                }
            } else {
                // Fallback for unexpected status (e.g., still stopped).
                _exit(UNKNOWN_ERROR); 
            }
            _exit(tracer_result);
        }
    } else {
        // MAIN PROCESS
        return;
    }
}

pid_t start_dummy_leader() {
    pid_t pid = fork();
    
    if (pid == -1) {
        return -1;
    }

    if (pid == 0) {
        if (setpgid(0, 0) == -1) {
            _exit(1);
        }
        for (int fd = 0; fd < 3; fd++) close(fd);
        char *cmd[] = {"sleep", "9999999", NULL}; 
        if (execvp(cmd[0], cmd) == -1) {
            _exit(127);
        }
    }
    
    return pid;
}

// Executes full pipeline of commands (series of commands, connected by pipes '|')
CommandExecutionResult_t* execute_commands_workflow(char** argv) {
    // it's a pipeline of commands
    pid_t pids[num_cmds]; // To store all child PIDs for later waiting

    // prev_pipe_read_fd holds the file descriptor for the read end of the pipe
    int prev_pipe_read_fd = STDIN_FILENO;

    // dummy leader of the process group will be used to kill all the processes in the group if necessary
    pid_t group_id = start_dummy_leader();

    int pipefd[2];
    bool is_last = (i >= num_cmds - 1);

    if (pipe(pipefd) == -1) {
        print_execution_error();
        // Attempt to clean up and continue waiting for launched children
        //clean_tokens(tokens, tokens_length);
        //clean_tokens(commands, num_cmds);
        return result;
    }
    trace_custom_command(
        tokens, tokens_length, result, 
        &pids[i], &prev_pipe_read_fd, pipefd,
        (i < num_cmds - 1), IS_NOT_SINGLE_COMMAND, group_id
    );
    if (pids[i] == -1) {
        if (prev_pipe_read_fd != STDIN_FILENO) 
            close(prev_pipe_read_fd);
        if (not(is_last)) { 
            close(pipefd[0]); 
            close(pipefd[1]); 
        }
        break;
    }
    if (prev_pipe_read_fd != STDIN_FILENO) {
        close(prev_pipe_read_fd);
    }

    if (not(is_last)) {
        // Parent closes its copy of the write end (pipefd[1])
        close(pipefd[1]);
        // Parent saves its copy of the read end to be the input for the next command
        prev_pipe_read_fd = pipefd[0];
    }

    if (prev_pipe_read_fd != STDIN_FILENO) {
        close(prev_pipe_read_fd); 
    }

    for (u_int32_t j = 0; j < num_cmds; j++) {
        wait_child_finish(pids[j]);
    }

    if (group_id > 0) {
        int status;
        // Use SIGKILL to terminate the sleep process quickly
        if (kill(group_id, SIGKILL) == -1 && errno != ESRCH) {
        }
        // Wait for it to be reaped to avoid zombies
        waitpid(group_id, &status, 0); 
    }
    clean_tokens(commands, num_cmds);
    return result;
}

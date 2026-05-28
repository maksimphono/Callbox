#include "utils.h"
#include "buildins.h"

int check_regex(char* str, const char* pattern) {
    regex_t regex;
    int reti;

    reti = regcomp(&regex, pattern, REG_EXTENDED);

    if (reti) {
        print_execution_error();
        return reti;
    }

    reti = regexec(&regex, str, 0, NULL, 0);

    regfree(&regex);

    return reti;
}

char process_escape_sequence(char* str) {
    if (IS_ESC_SEQ(*str)) {
        // it's realy an escape sequence
        char c = *(str + 1); // read the very next character
        switch(c) {
        case 'a':  return '\a';
        case 'b':  return '\b';
        case 'e':  return '\e';
        case 'f':  return '\f';
        case 'n':  return '\n';
        case 'r':  return '\r';
        case 't':  return '\t';
        case 'v':  return '\v';
        case '\\': return '\\';
        case '\'': return '\'';
        case '\"': return '\"';
        case '\?': return '\?';
        case ' ':  return ' ';
        default:   return '\0'; // invalid escape sequence
        }
    } else {
        return '\0'; // invalid escape sequence
    }
}

bool is_invalid_escape_sequence(char* str) {
    return ('\0' == process_escape_sequence(str));
}

char *strncpy_with_esc(char *dest, const char *src, size_t len) {
	char *tmp = dest;

	while (*src != '\0' && len > 0) {
        if (IS_ESC_SEQ(*src)) {
            //printf("Esc\n");
            *dest = process_escape_sequence((char*)src);
            if (*dest == '\0') {
                // invalid escape sequence
                print_invalid_syntax();
                return NULL;
            }
            *dest++;
            src += 2; // move over the excape sequence
        } else if (*src == PSEUDO_SP) {
            *dest++ = ' ';
            src++;
        } else
            *dest++ = *src++;

        len--;
    }
    
    *dest++ = '\0';
    return tmp;
}

char* replace_esc_seq(Token_t token) {
    return strcpy_with_esc(token, token);
}


char *strcpy_with_esc(char *dest, const char *src) {
    return strncpy_with_esc(dest, src, __UINT64_MAX__);
}

char* detect_sandbox(char* _raw_command) {
    u_int32_t len;
    char* raw_command = _raw_command;

    Token_t* tokens = tokenize_limit(_raw_command, &len, PSEUDO_SP, 2);
    if (strcmp(tokens[0], KEYWORD_SANDBOX) == 0) {
        // first instruction perris a "sandbox"

        if (len < 2) {
            clean_tokens(tokens, len);
            return NULL;
        }
        raw_command = _raw_command + strlen(tokens[0]) + strlen(tokens[1]) + 2;

        handle_buildin_sandbox(tokens, len);
    }

    clean_tokens(tokens, len);

    return raw_command;
}
// Same as regular 'execvp', but ignores all stdout redirections
int execvp_ignore_redirect(Token_t command, Token_t* tokens, u_int32_t tokens_length){
    u_int32_t length = 0;
    Token_t* tokens_without_redirect = (Token_t*)malloc((tokens_length + 1) * sizeof(Token_t));
    int result = 0;

    for (u_int32_t i = 0; tokens[i] != NULL; i++) {
        if (IS_NOT_STDOUT_REDIRECTION(tokens[i])) {
            tokens_without_redirect[length++] = tokens[i];
        }
    }
    tokens_without_redirect[length] = NULL;

    result = execvp(tokens_without_redirect[0], tokens_without_redirect);
    free(tokens_without_redirect);

    return result;
}

// Waits for child process to finish execution, analyzes exit code
// and print corresponding error message if necessary
void wait_child_finish(int pid) {
    int status;
    if (waitpid(pid, &status, 0) == -1 && errno != ECHILD) {
        print_execution_error();
    }

    if (WIFEXITED(status)) {
        // 2. Extract the actual exit code (0-255)
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0) {
            
            switch (exit_code) {
            case 127:
                print_command_not_found();
                break;
            default:
                print_execution_error();
                break;
            }
        }
    }
    // NOTE: don't know if this check is really necessary 
    else if (WIFSIGNALED(status)) {
        // The process was terminated by a signal (e.g., kill, Ctrl+C)
        if (status == SIGKILL) { // command was terminated because of blocked syscall in the pipeline
            
            return;
        }
        
        print_execution_error();
    }

    fflush(stdout);
}

// Finds all files (or devices) where stdout is redirected in the command, opens corresponding file descriptors and returns them in the array
int* find_stdout_redirect_targets(Token_t* tokens, u_int32_t tokens_length, u_int32_t* targets_num) {
    u_int32_t* ids = (u_int32_t*)malloc(tokens_length * sizeof(int));
    int* fds;
    u_int32_t ids_len = 0;

    for (u_int32_t i = 1; i < tokens_length; i++) { // assuming first token is the command
        if (IS_STDOUT_REDIRECTION(tokens[i])) {
            ids[ids_len++] = i;
        }
    }

    *targets_num = ids_len;

    fds = (int*)malloc(*targets_num * sizeof(int));

    for (u_int32_t i = 0; i < *targets_num; i++){
        Token_t current_target = tokens[ids[i]];
        fds[i] = open(current_target + 1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    free(ids);
    return fds;
}

// Writes message to current terminal session, 
// regardless of is process child or where it's stdout points to
void write_to_terminal(const char *message) {
    // 1. Open the controlling terminal device file.
    int tty_fd = open("/dev/tty", O_WRONLY);

    if (tty_fd == -1) {
        return;
    }

        ssize_t len = strlen(message);
        ssize_t bytes_written = write(tty_fd, message, len);

    if (bytes_written == -1 || bytes_written < len) {
    }
    
        close(tty_fd);
}

void init_environment() {
    char wd[PATH_MAX];
    if (getcwd(wd, sizeof(wd)) == NULL) {
        print_execution_error();
        exit(1);
    }

    if (clearenv() != 0) {
        print_execution_error();
        exit(1);
    }

    setenv("PATH", DEFAULT_ENV_PATH, 0);
    setenv("HOME", wd, 0);
    setenv("PWD", wd, 0);
    setenv("OLDPWD", wd, 0);
    setenv("LANG", DEFAULT_ENV_LANG, 0);
    setenv("ESH_VERSION", DEFAULT_ENV_VERSION, 0);

    // Initialize syscalls rules (all empty initially):
    init_syscall_rules();
}


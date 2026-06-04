#include "../include/utils.h"
#include "../include/print_message.h"

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
                // command not found
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


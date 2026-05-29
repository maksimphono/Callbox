#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <regex.h>
#include <sys/user.h> // For struct user_regs_struct
#include <sys/syscall.h>

#include <errno.h>
#include <stdbool.h>
#include "defs.h"
#include "tokenize.h"
#include "print_message.h"
#include "sandbox.h"


#define strtol_or_error(__str, __result, __on_error) \
do {\
    char* __endptr; \
    int32_t __value = strtol(__str, &__endptr, 10); \
    if ((errno == ERANGE && (__value == LONG_MAX || __value == LONG_MIN)) || __str == __endptr || *__endptr != '\0') {\
        __on_error \
    } \
    else { \
        __result = __value ; \
    } \
} while(false)

#define strtoll_or_error(__str, __result, __on_error) \
do {\
    char* __endptr; \
    int64_t __value = strtoll(__str, &__endptr, 10); \
    if ((errno == ERANGE && (__value == LLONG_MAX || __value == LLONG_MIN)) || __str == __endptr || *__endptr != '\0') {\
        __on_error \
    } \
    else { \
        __result = __value ; \
    } \
} while(false)

#define strtoull_or_error(__str, __result, __on_error) \
do {\
    char* __endptr; \
    u_int64_t __value = strtoull(__str, &__endptr, 10); \
    if ((errno == ERANGE && (__value == ULLONG_MAX)) || __str == __endptr || *__endptr != '\0') {\
        __on_error \
    } \
    else { \
        __result = __value ; \
    } \
} while(false)

#define strtoul_or_error(__str, __result, __on_error) \
do {\
    char* __endptr; \
    u_int32_t __value = strtoul(__str, &__endptr, 10); \
    if ((errno == ERANGE && (__value == ULONG_MAX)) || __str == __endptr || *__endptr != '\0') {\
        __on_error \
    } \
    else { \
        __result = __value ; \
    } \
} while(false)

int check_regex(char* str, const char* pattern);

char process_escape_sequence(char* str);

bool is_invalid_escape_sequence(char* str);

char *strncpy_with_esc(char *dest, const char *src, size_t len);
char *strcpy_with_esc(char *dest, const char *src);

char* replace_esc_seq(Token_t token);

// Waits for child process to finish execution, analyzes exit code
// and print corresponding error message if necessary
void wait_child_finish(int pid);

// Will set rules for the syscall by provided name
void cleanup(char* raw_command);

#endif
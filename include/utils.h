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
#include "sandbox.h"

#define realloc_or_err(__ptr, __size, __on__error) ({ \
    void* __local_temp = realloc(__ptr, __size); \
    if (__local_temp == NULL) { \
        __on__error \
    } else { \
        (__ptr) = __local_temp; \
    } \
    __local_temp; \
})


#define strtohex_or_error(__str, __result, __on_error) \
do {\
    char* __endptr; \
    int32_t __value = strtol(__str, &__endptr, 16); \
    if ((errno == ERANGE && (__value == LONG_MAX || __value == LONG_MIN)) || __str == __endptr || *__endptr != '\0') {\
        __on_error \
    } \
    else { \
        __result = __value ; \
    } \
} while(false)

#define strtol_or_err(__str, __on_error) ({\
    const char* __local__str = (__str); \
    char* __endptr; \
    errno = 0; \
    long __value = strtol(__local__str, &__endptr, 10); \
    if ((errno == ERANGE && (__value == LONG_MAX || __value == LONG_MIN)) || __local__str == __endptr || *__endptr != '\0') {\
        __on_error \
    } \
    __value; \
})

#define strtoul_or_err(__str, __on_error) ({\
    const char* __local__str = (__str); \
    char* __endptr; \
    errno = 0; \
    unsigned long __value = strtoul(__local__str, &__endptr, 10); \
    if ((errno == ERANGE && (__value == LONG_MAX || __value == LONG_MIN)) || __local__str == __endptr || *__endptr != '\0') {\
        __on_error \
    } \
    __value; \
})

#define strtoll_or_err(__str, __on_error) ({\
    const char* __local__str = (__str); \
    char* __endptr; \
    errno = 0; \
    long long __value = strtoll(__local__str, &__endptr, 10); \
    if ((errno == ERANGE && (__value == LONG_MAX || __value == LONG_MIN)) || __local__str == __endptr || *__endptr != '\0') {\
        __on_error \
    } \
    __value; \
})

#define strtoull_or_err(__str, __on_error) ({\
    const char* __local__str = (__str); \
    char* __endptr; \
    errno = 0; \
    unsigned long long __value = strtol(__local__str, &__endptr, 10); \
    if ((errno == ERANGE && (__value == LONG_MAX || __value == LONG_MIN)) || __local__str == __endptr || *__endptr != '\0') {\
        __on_error \
    } \
    __value; \
})


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
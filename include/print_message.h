#ifndef _PRINT_MESSAGE_H_
#define _PRINT_MESSAGE_H_

#include <stdarg.h>
#include <stdio.h>

#include "sandbox.h"

void print_missing_tracee();

void print_empty_rules();

void print_invalid_syntax();

void print_execution_error();

void print_catched_syscall(int, const char*, char* syscall_name, u_int8_t count, Syscall_argument args[MAX_SYSCALL_ARGS_NUM]);

#endif
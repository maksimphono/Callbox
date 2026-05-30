#ifndef _PRINT_MESSAGE_H_
#define _PRINT_MESSAGE_H_

#include <stdarg.h>
#include <stdio.h>

void print_missing_tracee();

void print_empty_rules();

void print_invalid_syntax();

void print_execution_error();

void print_catched_syscall(const char*, char* syscall_name, int count, ...);

#endif
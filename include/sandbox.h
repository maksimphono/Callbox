#ifndef _SANDBOX_H_
#define _SANDBOX_H_

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "defs.h"
#include "print_message.h"
#include "utils.h"


typedef enum {SUCCESS, UNKNOWN_ERROR, BLOCKED_SYSCALL, FILTERED_SYSCALL, NOTIFIED_SYSCALL} Trace_result;
typedef enum {
    ___NONE_TYPE = 0,
    INT_32_TYPE  = 1, 
    UINT_32_TYPE = 2, 
    INT_64_TYPE  = 3, 
    UINT_64_TYPE = 4, 
    STRING_TYPE  = 5, 
    ADDRESS_TYPE = 6, 
    OTHER_TYPE
} Syscall_arg_type;

typedef enum {
    NONE_ACTION = 0,
    BLOCK, // immediately stop execution of the program before that syscall
    FILTER, // don't execute this syscall, continue executing the program
    NOTIFY // just notify about syscall being made with specific arguments, continue execution
} Action_type;

// Structure, that will hold rules for each syscall, each element is a syscall with list of corresponding forbidden agruments values

enum {
    SYS_READ        = 0,
    SYS_WRITE       = 1,
    SYS_OPEN        = 2,
    SYS_MMAP        = 9,
    SYS_PIPE        = 22,
    SYS_SCHED_YIELD = 24,
    SYS_DUP         = 32,
    SYS_CLONE       = 56,
    SYS_FORK        = 57,
    SYS_EXECVE      = 59,
    SYS_MKDIR       = 83,
    SYS_CHMOD       = 90
};

// Argument of a syscall, that will be stored in syscall rules list and define undesired argument type and it's value
typedef struct Syscall_argument {
    Syscall_arg_type type;
    union {
        u_int64_t uint64;
        int64_t int64;
        u_int32_t uint32;
        int32_t int32;
        u_int16_t uint16;
        int16_t int16;
        struct {
            bool is_regex;
            char* str;
        };
        uintptr_t addr[2];
        void* other;
    };
} Syscall_argument;

typedef struct Syscall_abstract {
    u_int32_t number;
    const char* name;
    const Syscall_arg_type arg_types[6];
    Syscall_argument* rules;
    Action_type action;
} Syscall_abstract;

extern const Syscall_argument EMPTY_RULES;

#define RULE_IS_NONE(rule) (rule.type == ___NONE_TYPE)
#define EMPTY_SYSCALL {0, NULL, {___NONE_TYPE,___NONE_TYPE,___NONE_TYPE,___NONE_TYPE,___NONE_TYPE,___NONE_TYPE},NULL,NONE_ACTION}
#define IS_EMPTY_SYSCALL(syscall) (syscall.name == NULL)

typedef struct Node_recorded_rules_t {u_int32_t syscall_num; struct Node_recorded_rules_t* next;} Node_recorded_rules_t;

void init_syscall_rules();

void del_syscall_rules();

bool check_rules(struct user_regs_struct regs);

bool cmp_syscall_argument(Syscall_argument argument, reg_t value, char*);

void record_syscall_with_rules(u_int32_t syscall_num);

Syscall_argument* get_rules_for_syscall_id(u_int32_t id);

const char* get_name_for_syscall_id(u_int32_t id);

const Syscall_arg_type* get_syscall_argument_types(reg_t syscall_num);

void clean_syscall_rules();

void reset_syscall_rules();

// Will set rules for the syscall by provided name
void set_rules_for_syscall_name(char* name, char** arguments, u_int32_t arguments_length, Action_type);

Action_type print_blocked_syscall_arguments(reg_t syscall_num, pid_t pid, struct user_regs_struct regs, int);

#endif
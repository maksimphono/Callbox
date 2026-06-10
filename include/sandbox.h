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
#include "utils.h"

typedef unsigned char flags8_t;

typedef enum {SUCCESS, UNKNOWN_ERROR, BLOCKED_SYSCALL} Trace_result;
typedef enum {
    ___NONE_TYPE = 0,
    INT_32_TYPE  = 1, 
    UINT_32_TYPE = 2, 
    INT_64_TYPE  = 3, 
    UINT_64_TYPE = 4, 
    STRING_TYPE  = 5, 
    ADDRESS_TYPE = 6,
    ARRAY_TYPE   = 7,
    VARIAD_TYPE  = 8,
    OTHER_TYPE
} Syscall_arg_type;

typedef enum {
    NONE_ACTION = 0,
    BLOCK, // immediately stop execution of the program before that syscall
    FILTER, // don't execute this syscall, continue executing the program
    NOTIFY // just notify about syscall being made with specific arguments, continue execution
} Action_type;

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
        struct {
            unsigned char* arr;
            size_t arr_len;
        };
        uintptr_t addr[2];
        void* other;
    };
} Syscall_argument;

typedef struct Syscall_abstract {
    u_int32_t number;
    const char name[SYSCALL_NAME_MAX_LEN];
    const Syscall_arg_type arg_types[MAX_SYSCALL_ARGS_NUM];
    unsigned char flags;  // 7th bit -> is_special | none | none | none | none | none | none | none <- 0th bit
    Syscall_argument* rules;
    Action_type action;
} Syscall_abstract;

extern const Syscall_argument EMPTY_RULES;

#define IS_SPECIAL(__flags) ((__flags & (flags8_t)128) == (flags8_t)128)
#define RULE_IS_NONE(rule) (rule.type == ___NONE_TYPE)
#define EMPTY_SYSCALL {0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, {___NONE_TYPE,___NONE_TYPE,___NONE_TYPE,___NONE_TYPE,___NONE_TYPE,___NONE_TYPE}, 0, NULL,NONE_ACTION}
#define IS_EMPTY_SYSCALL(syscall) (syscall.name[0] == 0)

typedef struct Node_recorded_rules_t {u_int32_t syscall_num; struct Node_recorded_rules_t* next;} Node_recorded_rules_t;

void init_syscall_rules();

void del_syscall_rules();

bool check_rules(struct user_regs_struct regs);

bool cmp_syscall_argument(Syscall_argument* argument, Syscall_argument*);

void record_syscall_with_rules(u_int32_t syscall_num);

Syscall_argument* get_rules_for_syscall_id(u_int32_t id);

const char* get_name_for_syscall_id(u_int32_t id);

const Syscall_arg_type* get_syscall_argument_types(reg_t syscall_num);

void clean_arguments(Syscall_argument* arguments);

void clean_syscall_rules();

void reset_syscall_rules();

 
byte_t* read_data_from_tracee(pid_t pid, reg_t addr, size_t length);

char* read_str_from_tracee(pid_t pid, reg_t addr);

// Will set rules for the syscall by provided name
ExitStatus_t set_rules_for_syscall_name(char* name, Syscall_argument arguments[], u_int32_t arguments_length, Action_type);

u_int8_t process_special_syscall(reg_t syscall_num, pid_t pid, const reg_t arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument received_args[MAX_SYSCALL_ARGS_NUM]);

Action_type print_blocked_syscall_arguments(reg_t syscall_num, pid_t pid, struct user_regs_struct regs, int);

#endif
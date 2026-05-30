#ifndef _CLI_ARGUMENTS_H_
#define _CLI_ARGUMENTS_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include "argparse.h"
#include "print_message.h"


#define PROGRAM_DESCRIPTION "A simple program, that can trace syscalls, made by another program"

typedef struct {
    const char* input_file;
    const char* rules_file;
    const char* trace_output_file;
} Arguments;

extern Arguments cli_arguments;

Arguments* scan_cli_arguments(u_int32_t* , u_int32_t argc, const char** argv);

#endif
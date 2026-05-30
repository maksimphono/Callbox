#include "../include/cli_arguments.h"

Arguments cli_arguments = {
    NULL, NULL, NULL
};

Arguments* scan_cli_arguments(u_int32_t* arg_num, u_int32_t argc, const char** argv) {
    static const char *const usages[] = {
        "basic [options] [[--] args]",
        "basic [options]",
        NULL,
    };

    u_int32_t args_end_index = 0;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_STRING('r', "rules", &cli_arguments.rules_file, "path to rules file", NULL, 0, 0),
        OPT_STRING('i', "input", &cli_arguments.input_file, "path to input file", NULL, 0, 0),
        OPT_STRING('o', "output", &cli_arguments.trace_output_file, "path to output file", NULL, 0, 0),
        OPT_END(),
    };

    // locating argument '--', after that tracee will be specified
    for (args_end_index = 0; args_end_index < argc; args_end_index++) {
        if (strncmp("--", argv[args_end_index], 3) == 0) {
            break;
        }
    }

    if (args_end_index == argc) {
        print_missing_tracee();
        return NULL;
    }

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, PROGRAM_DESCRIPTION, "");
    argparse_parse(&argparse, args_end_index, argv);

    *arg_num = args_end_index;

    return &cli_arguments;
}
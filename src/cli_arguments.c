#include "../include/cli_arguments.h"

Arguments cli_arguments = {
    NULL, NULL, NULL
};

Arguments* scan_cli_arguments(int argc, const char** argv) {
    static const char *const usages[] = {
        "basic [options] [[--] args]",
        "basic [options]",
        NULL,
    };

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_STRING('r', "rules", &cli_arguments.rules_file, "path to rules file", NULL, 0, 0),
        OPT_STRING('i', "input", &cli_arguments.input_file, "path to input file", NULL, 0, 0),
        OPT_STRING('o', "output", &cli_arguments.trace_output_file, "path to output file", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, PROGRAM_DESCRIPTION, "");
    argc = argparse_parse(&argparse, argc, argv);

    printf("Rules: %s, Inp: %s, Out: %s\n", cli_arguments.rules_file, cli_arguments.input_file, cli_arguments.trace_output_file);

    return &cli_arguments;
}
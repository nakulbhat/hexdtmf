#include <stdio.h>

#define OPTIONS_STRING "\t\t%-4s %-12s %s\n"

void emit_help(const char *prog_name) {
    fprintf(stderr, "Usage: %s <commands> [OPTIONS]\n", prog_name);
    fputs("\n", stderr);
    fputs("\tCOMMANDS\n", stderr);
    fputs("\t\tencode [DIGIT_STRING] [OUTPUT_FILE] [OPTIONS]\n", stderr);
    fputs("\t\tdecode [INPUT_FILE] [OPTIONS]\n", stderr);
    fputs("\n", stderr);
    fputs("\tOPTIONS\n", stderr);
    fprintf(stderr, OPTIONS_STRING, "-h,", "--help,",
            "print this help screen");
    fprintf(stderr, OPTIONS_STRING, "-v,", "--verbose,",
            "print verbose output");
    fprintf(stderr, OPTIONS_STRING, "-t,", "--tone,",
            "tone duration in milliseconds");
    fprintf(stderr, OPTIONS_STRING, "-g,", "--gap,",
            "gap duration in milliseconds");
    fprintf(stderr, OPTIONS_STRING, "-o,", "--output,",
            "specify output filename");
}

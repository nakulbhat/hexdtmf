#include <stdio.h>

#define OPTIONS_STRING "\t\t%-10s %-22s %s\n"

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
    fprintf(stderr, OPTIONS_STRING, "-t <ms>,", "--tone <ms>,",
            "tone duration in milliseconds");
    fprintf(stderr, OPTIONS_STRING, "-g <ms>,", "--gap <ms>,",
            "gap duration in milliseconds");
    fprintf(stderr, OPTIONS_STRING, "-o <file>,", "--output <file>,",
            "specify output filename");
    fprintf(stderr, OPTIONS_STRING, "-i <file>,", "--input <file>,",
            "specify input filename");
    fprintf(stderr, OPTIONS_STRING, "-c,", "--compat,",
            "non-hexadecimal mode, use * and #");
    fprintf(stderr, "\t\t%-33s %s\n", "", "instead of 0xE and 0xF respectively");

}

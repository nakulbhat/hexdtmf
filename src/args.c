#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/help.h"
#include "../include/main.h"

void set_tone_duration(int argc, char **argv, int location) {
    if (flags & TONE_DURATION_FLAG) {
        fputs("Tone duration cannot be specified twice\n", stderr);
        exit(EXIT_FAILURE);
    }

    if (location + 1 >= argc){
        fputs("Tone flag passed but no duration specified\n", stderr);
        exit(EXIT_FAILURE);
    }

    flags |= TONE_DURATION_FLAG;

    char *str_end;
    int milliseconds = strtol(argv[location + 1], &str_end, 10);

    tone_duration_ms = milliseconds;
    LOG("Tone duration set to %d milliseconds\n", milliseconds);
}

void set_gap_duration(int argc, char **argv, int location) {
    if (flags & GAP_DURATION_FLAG){
        fputs("Gap duration cannot be specified twice\n", stderr);
        exit(EXIT_FAILURE);
    }
    if (location + 1 >= argc){
        fputs("Gap flag passed but no duration specified\n", stderr);
        exit(EXIT_FAILURE);
    }

    flags |= GAP_DURATION_FLAG;

    char *str_end;
    int milliseconds = strtol(argv[location + 1], &str_end, 10);
    gap_duration_ms = milliseconds;
    LOG("Gap duration set to %d milliseconds\n", milliseconds);
}

void parse_args(int argc, char **argv) {
    if (argc < 2) {
        emit_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse the mode of operation
    if (strcmp(argv[1], "encode") == 0)
        flags |= ENCODE_FLAG;
    else if (strcmp(argv[1], "decode") == 0)
        flags |= DECODE_FLAG;
    else if (strcmp(argv[1], "--help") == 0){
            emit_help(argv[0]);
            exit(EXIT_SUCCESS);
        }
    else {
        puts("Unrecognized mode specified. Please check your command.");
        emit_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Check remaining flags for -v or -h
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0){
                flags |= VERBOSE_FLAG;
            LOG("Verbose mode activated\n");
                }
        else if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)){
            emit_help(argv[0]);
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tone") == 0)
            set_tone_duration(argc, argv, i);
        else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--gap") == 0)
            set_gap_duration(argc, argv, i);

    }

}

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/args.h"
#include "../include/help.h"
#include "../include/main.h"

void set_tone_duration(int argc, char **argv, int location) {
    if (flags & TONE_DURATION_FLAG) {
        fputs("Tone duration cannot be specified twice\n", stderr);
        exit(EXIT_FAILURE);
    }

    if (location + 1 >= argc) {
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
    if (flags & GAP_DURATION_FLAG) {
        fputs("Gap duration cannot be specified twice\n", stderr);
        exit(EXIT_FAILURE);
    }
    if (location + 1 >= argc) {
        fputs("Gap flag passed but no duration specified\n", stderr);
        exit(EXIT_FAILURE);
    }

    flags |= GAP_DURATION_FLAG;

    char *str_end;
    int milliseconds = strtol(argv[location + 1], &str_end, 10);
    gap_duration_ms = milliseconds;
    LOG("Gap duration set to %d milliseconds\n", milliseconds);
}

void set_output_filename(int argc, char **argv, int location) {
    if (flags & OUTPUT_FILENAME_FLAG) {
        fputs("Output file cannot be specified twice\n", stderr);
        exit(EXIT_FAILURE);
    }
    if (location + 1 >= argc) {
        fputs("Output filename flag passed but no filename specified\n", stderr);
        exit(EXIT_FAILURE);
    }

    flags |= OUTPUT_FILENAME_FLAG;

    output_filename = strdup(argv[location + 1]);
    LOG("Output filename set to %s\n", output_filename);
}

void set_input_filename(int argc, char **argv, int location) {
    if (flags & INPUT_FILENAME_FLAG) {
        fputs("Input file cannot be specified twice\n", stderr);
        exit(EXIT_FAILURE);
    }
    if (location + 1 >= argc) {
        fputs("Input filename flag passed but no filename specified\n", stderr);
        exit(EXIT_FAILURE);
    }

    flags |= INPUT_FILENAME_FLAG;

    input_filename = strdup(argv[location + 1]);
    LOG("Input filename set to %s\n", input_filename);
}

void set_encoding_string(const char *string) {
    if (flags & INPUT_FILENAME_FLAG) {
        fputs("Input file cannot be specified alongside encoding string\n", stderr);
        exit(EXIT_FAILURE);
    }
    if (flags & ENCODING_STRING_FLAG) {
        fputs("Encoding string cannot be specified twice\n", stderr);
    }
    flags |= ENCODING_STRING_FLAG;

    encoding_string = strdup(string);
    LOG("Encoding string set to %s\n", encoding_string);
}

void parse_args(int argc, char **argv) {
    if (argc < 2) {
        emit_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse the mode of operation
    if (ACMP(argv[1], "encode"))
        flags |= ENCODE_FLAG;
    else if (ACMP(argv[1], "decode"))
        flags |= DECODE_FLAG;
    else if (ACMP(argv[1], "--help")) {
        emit_help(argv[0]);
        exit(EXIT_SUCCESS);
    } else {
        puts("Unrecognized mode specified. Please check your command.");
        emit_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Check remaining flags for -v or -h
    for (int i = 2; i < argc; i++) {
        if (ACMP(argv[i], "-v") || ACMP(argv[i], "--verbose")) {
            flags |= VERBOSE_FLAG;
            LOG("Verbose mode activated\n");
        } else if (ACMP(argv[i], "-h") || ACMP(argv[i], "--help")) {
            emit_help(argv[0]);
            exit(EXIT_SUCCESS);
        } else if (ACMP(argv[i], "-c") || ACMP(argv[i], "--compat")) {
            flags |= COMPAT_FLAG;
            LOG("Compatibility mode activated\n");
        }
    }

    for (int i = 1; i < argc; i++) {
        if (ACMP(argv[i], "-t") || ACMP(argv[i], "--tone"))
            set_tone_duration(argc, argv, i++);
        else if (ACMP(argv[i], "-g") || ACMP(argv[i], "--gap"))
            set_gap_duration(argc, argv, i++);
        else if (ACMP(argv[i], "-o") || ACMP(argv[i], "--output"))
            set_output_filename(argc, argv, i++);
        else if (ACMP(argv[i], "-i") || ACMP(argv[i], "--input"))
            set_input_filename(argc, argv, i++);
            // Passthrough conditions.
        else if (ACMP(argv[i], "-v") || ACMP(argv[i], "--verbose"))
            ;
        else if (ACMP(argv[i], "decode") || ACMP(argv[i], "encode"))
            ;
        else if (ACMP(argv[i], "-c") || ACMP(argv[i], "--compat"))
            ;

        else if (!(flags & ENCODING_STRING_FLAG) && !(flags & INPUT_FILENAME_FLAG))
            set_encoding_string(argv[i]);
        else {
            fprintf(stderr, "Unable to parse argument %s\n", argv[i]);
            emit_help(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

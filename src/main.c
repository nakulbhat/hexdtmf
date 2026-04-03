#include <stddef.h>
#include <stdio.h>

#include "../include/help.h"
#include "../include/main.h"
#include "../include/args.h"
#include "../include/encode.h"

FLAGS flags = 0;

char *output_filename = "output.wav";
int tone_duration_ms = 50;
int gap_duration_ms = 10;


int main(int argc, char **argv) {
    parse_args(argc, argv);

    if (flags & ENCODE_FLAG) encode("124921");
    if (flags & DECODE_FLAG) decode();
    return 0;
}

void decode() { puts("Decoding mode."); }

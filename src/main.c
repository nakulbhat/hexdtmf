#include <stddef.h>
#include <stdio.h>

#include "../include/args.h"
#include "../include/encode.h"
#include "../include/help.h"
#include "../include/main.h"
#include "../include/decoder.h"

FLAGS flags = 0;

char *output_filename = "output.wav";
char *input_filename = NULL;
char *encoding_string = NULL;
int tone_duration_ms = 50;
int gap_duration_ms = 10;

int main(int argc, char **argv) {
    parse_args(argc, argv);
    if (flags & ENCODE_FLAG) encoder();
    if (flags & DECODE_FLAG) decoder();
    return 0;
}


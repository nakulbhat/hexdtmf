#include <stdio.h>

#define OPT(fmt, desc) fprintf(stderr, "  %-22s %s\n", fmt, desc)

void emit_help(const char *prog_name) {
    fprintf(stderr,
            "hexdtmf — Encode and decode DTMF tones (WAV)\n"
            "\n"
            "USAGE:\n"
            "  %s encode [STRING] [OPTIONS]\n"
            "  %s decode [OPTIONS]\n"
            "\n",
            prog_name, prog_name);

    fprintf(stderr, "DESCRIPTION:\n");
    fprintf(stderr,
            "  hexdtmf encodes digit strings into DTMF WAV audio, or decodes\n"
            "  DTMF tones from WAV files back into digit strings.\n"
            "\n"
            "  Supported digits:\n"
            "    0-9, A-D, E, F (hex keypad)\n"
            "\n"
            "  Compatibility mode (-c) maps:\n"
            "    '*' -> E\n"
            "    '#' -> F\n"
            "\n");

    fprintf(stderr, "COMMANDS:\n");
    fprintf(stderr, "  encode            Encode digits into WAV audio\n"
            "  decode            Decode WAV audio into digits\n"
            "\n");

    fprintf(stderr, "OPTIONS:\n");
    OPT("-h, --help", "Show this help message");
    OPT("-v, --verbose", "Enable verbose logging");
    OPT("-c, --compat", "Enable * and # compatibility mode");
    OPT("-t, --tone <ms>", "Tone duration in milliseconds (default: 50)");
    OPT("-g, --gap <ms>", "Gap duration in milliseconds (default: 10)");
    OPT("-o, --output <file>", "Output file (default: output.wav)");
    OPT("-i, --input <file>", "Input file (default: stdin)");
    fprintf(stderr, "\n");

    fprintf(stderr, "ENCODE INPUT:\n");
    fprintf(stderr,
            "  Digits may be provided in three ways:\n"
            "\n"
            "  1. Command line string\n"
            "       %s encode 123ABC\n"
            "\n"
            "  2. Input file\n"
            "       %s encode -i digits.txt\n"
            "\n"
            "  3. Standard input\n"
            "       echo 123ABC | %s encode\n"
            "\n",
            prog_name, prog_name, prog_name);

    fprintf(stderr, "DECODE INPUT:\n");
    fprintf(stderr,
            "  Decode from WAV file:\n"
            "       %s decode -i tones.wav\n"
            "\n"
            "  Decode from stdin:\n"
            "       cat tones.wav | %s decode\n"
            "\n",
            prog_name, prog_name);

    fprintf(stderr, "EXAMPLES:\n");
    fprintf(stderr,
            "  Encode digits to WAV\n"
            "       %s encode 123ABCD -o tones.wav\n"
            "\n"
            "  Encode using custom timing\n"
            "       %s encode 1234 -t 100 -g 20\n"
            "\n"
            "  Decode WAV file\n"
            "       %s decode -i tones.wav\n"
            "\n"
            "  Decode with compatibility mode\n"
            "       %s decode -c -i tones.wav\n"
            "\n",
            prog_name, prog_name, prog_name, prog_name);

    fprintf(stderr,
            "NOTES:\n"
            "  + Decoder automatically detects tone/gap duration when possible\n"
            "  + Auto-detection requires tone and gap durations > 5 ms.\n"
            "    Use -t and -g to specify smaller values.\n"
            "  + Only 16-bit PCM WAV files are supported\n"
            "  + Stereo files are automatically mixed to mono\n"
            "\n");
}

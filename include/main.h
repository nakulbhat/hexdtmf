#ifndef MAIN
#define MAIN
void decode();

typedef enum {
    ENCODE_FLAG = 1 << 0,
    DECODE_FLAG = 1 << 1,
    FILENAME_FLAG = 1 << 2,
    TONE_DURATION_FLAG = 1 << 3,
    GAP_DURATION_FLAG = 1 << 4,
    VERBOSE_FLAG = 1 << 5
} FLAGS;

extern FLAGS flags;

extern char *output_filename;
extern int tone_duration_ms;
extern int gap_duration_ms;

#define LOG(...) do {if (flags & VERBOSE_FLAG) fprintf(stderr, __VA_ARGS__);} while(0)

#endif // MAIN

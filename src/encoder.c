#include "../include/main.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SAMPLE_RATE 8000
#define AMPLITUDE 16000
#define PI 3.14159265358979323846

typedef struct {
    char key;
    int low_freq;
    int high_freq;
} DTMFTone;

static const DTMFTone dtmf_table[] = {
    {'1', 697, 1209}, {'2', 697, 1336}, {'3', 697, 1477}, {'A', 697, 1633},
    {'4', 770, 1209}, {'5', 770, 1336}, {'6', 770, 1477}, {'B', 770, 1633},
    {'7', 852, 1209}, {'8', 852, 1336}, {'9', 852, 1477}, {'C', 852, 1633},
    {'E', 941, 1209}, {'0', 941, 1336}, {'F', 941, 1477}, {'D', 941, 1633},
    {0, 0, 0}};

int dtmf_freqs(char key, int *low, int *high) {
    for (int i = 0; dtmf_table[i].key != 0; i++) {
        if (dtmf_table[i].key == key) {
            *low = dtmf_table[i].low_freq;
            *high = dtmf_table[i].high_freq;
            return 1;
        }
    }
    return 0;
}

void dtmf_generate(short *buf, int num_samples, int low_freq, int high_freq) {
    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        double sample =
            AMPLITUDE * 0.5 *
            (sin(2.0 * PI * low_freq * t) + sin(2.0 * PI * high_freq * t));
        buf[i] = (short)sample;
    }
}

void write_wav(const char *filename, short *samples, int num_samples) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        return;
    }

    int data_size = num_samples * sizeof(short);
    int chunk_size = 36 + data_size;
    short num_ch = 1;
    int byte_rate = SAMPLE_RATE * sizeof(short);
    short block_align = sizeof(short);
    short bits = 16;
    int sample_rate = SAMPLE_RATE;

    fwrite("RIFF", 1, 4, f);
    fwrite(&chunk_size, 4, 1, f);
    fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f);
    int fmt_size = 16;
    short pcm = 1;
    fwrite(&fmt_size, 4, 1, f);
    fwrite(&pcm, 2, 1, f);
    fwrite(&num_ch, 2, 1, f);
    fwrite(&sample_rate, 4, 1, f);
    fwrite(&byte_rate, 4, 1, f);
    fwrite(&block_align, 2, 1, f);
    fwrite(&bits, 2, 1, f);
    fwrite("data", 1, 4, f);
    fwrite(&data_size, 4, 1, f);
    fwrite(samples, sizeof(short), num_samples, f);
    fclose(f);
}

static char *apply_compat(const char *digits) {
    char *out = strdup(digits);
    if (!out)
        return NULL;
    for (char *p = out; *p; p++) {
        if (*p == '*')
            *p = 'E';
        else if (*p == '#')
            *p = 'F';
    }
    return out;
}
static char *build_encoding_string(void) {
    FILE *f = fopen(input_filename, "r");
    if (!f) {
        fprintf(stderr, "Cannot open input file: %s\n", input_filename);
        return NULL;
    }

    /* Measure file size for a safe upper-bound allocation */
    if (fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "fseek failed on: %s\n", input_filename);
        fclose(f);
        return NULL;
    }
    long file_size = ftell(f);
    if (file_size < 0) {
        fprintf(stderr, "ftell failed on: %s\n", input_filename);
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buf = malloc(file_size + 1);
    if (!buf) {
        fputs("malloc failed\n", stderr);
        fclose(f);
        return NULL;
    }

    /* Read line by line, strip spaces and newlines */
    size_t n = 0;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == ' ' || c == '\r' || c == '\n' || c == '\t')
            continue;
        buf[n++] = (char)c;
    }
    buf[n] = '\0';
    fclose(f);

    if (n == 0) {
        fputs("Input file is empty or contains only whitespace.\n", stderr);
        free(buf);
        return NULL;
    }

    LOG("build_encoding_string: built %zu-char string from %s\n", n,
        input_filename);
    return buf;
}

static char *read_stdin_string(void) {
    size_t cap = 1024;
    size_t len = 0;

    char *buf = malloc(cap);
    if (!buf) {
        fputs("malloc failed\n", stderr);
        return NULL;
    }

    int c;
    while ((c = fgetc(stdin)) != EOF) {
        if (c == ' ' || c == '\r' || c == '\n' || c == '\t')
            continue;

        if (len + 1 >= cap) {
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if (!tmp) {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }

        buf[len++] = toupper((unsigned char)c);
    }

    if (len == 0) {
        free(buf);
        return NULL;
    }

    buf[len] = '\0';
    return buf;
}

static int encode(const char *digits) {
    int tone_samples = (int)((double)tone_duration_ms / 1000.0 * SAMPLE_RATE);
    int gap_samples = (int)((double)gap_duration_ms / 1000.0 * SAMPLE_RATE);

    int valid = 0;
    for (const char *p = digits; *p; p++) {
        int low, high;
        if (dtmf_freqs(*p, &low, &high))
            valid++;
    }
    if (valid == 0) {
        fputs("No valid DTMF digits found.\n", stderr);
        return 1;
    }

    int total_samples = valid * (tone_samples + gap_samples) - gap_samples;
    short *buf = calloc(total_samples, sizeof(short));
    if (!buf) {
        fputs("malloc failed\n", stderr);
        return 1;
    }

    int offset = 0;
    for (const char *p = digits; *p; p++) {
        int low, high;
        if (!dtmf_freqs(*p, &low, &high))
            continue;
        dtmf_generate(buf + offset, tone_samples, low, high);
        LOG("Encoded '%c'  offset=%-6d  (%d Hz + %d Hz)\n", *p, offset, low, high);
        offset += tone_samples;
        if (offset < total_samples)
            offset += gap_samples;
    }

    write_wav(output_filename, buf, total_samples);
    LOG("Written %d samples to %s\n", total_samples, output_filename);
    free(buf);
    return 0;
}

/*
 * Public entry point. Resolves the digit source (global string or file),
 * then delegates to encode().
 */
int encoder(void) {

    if (!(flags & ENCODE_FLAG)) {
        fputs("encoder: ENCODE_FLAG not set.\n", stderr);
        return 1;
    }

    /* Prefer explicit encoding string over file input */
    if (encoding_string != NULL) {
        LOG("encoder: using encoding_string \"%s\"\n", encoding_string);
        if (flags & COMPAT_FLAG) {
            char *translated =
                apply_compat(encoding_string); // src = encoding_string or file digits
            int result = encode(translated);
            free(translated);
            return result;
        } else {
            return encode(encoding_string);
        }
    }

    if (flags & INPUT_FILENAME_FLAG) {
        char *digits = build_encoding_string();
        if (!digits)
            return 1;
        int result;
        if (flags & COMPAT_FLAG) {
            char *translated = apply_compat(digits);   // src = encoding_string or file digits
            result = encode(translated);
            free(translated);
        } else {
            result = encode(digits);
        }
        free(digits);
        return result;
    }

    /* fallback to stdin */
    char *digits = read_stdin_string();
    if (!digits) {
        fputs("encoder: no input specified.\n", stderr);
        return 1;
    }

    int result;
    if (flags & COMPAT_FLAG) {
        char *translated = apply_compat(digits);
        result = encode(translated);
        free(translated);
    } else {
        result = encode(digits);
    }

    free(digits);
    return result;
}

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/main.h"

#define SAMPLE_RATE  8000
#define AMPLITUDE    16000
#define PI           3.14159265358979323846


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
    {0, 0, 0}
};

int dtmf_freqs(char key, int *low, int *high) {
    for (int i = 0; dtmf_table[i].key != 0; i++) {
        if (dtmf_table[i].key == key) {
            *low  = dtmf_table[i].low_freq;
            *high = dtmf_table[i].high_freq;
            return 1;
        }
    }
    return 0;
}

void dtmf_generate(short *buf, int num_samples, int low_freq, int high_freq) {
    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        double sample = AMPLITUDE * 0.5 * (
            sin(2.0 * PI * low_freq  * t) +
            sin(2.0 * PI * high_freq * t)
        );
        buf[i] = (short)sample;
    }
}

void write_wav(const char *filename, short *samples, int num_samples) {
    FILE *f = fopen(filename, "wb");
    if (!f) { perror("fopen"); return; }

    int   data_size   = num_samples * sizeof(short);
    int   chunk_size  = 36 + data_size;
    short num_ch      = 1;
    int   byte_rate   = SAMPLE_RATE * sizeof(short);
    short block_align = sizeof(short);
    short bits        = 16;
    int   sample_rate = SAMPLE_RATE;

    fwrite("RIFF",        1, 4, f);
    fwrite(&chunk_size,   4, 1, f);
    fwrite("WAVE",        1, 4, f);
    fwrite("fmt ",        1, 4, f);
    int fmt_size = 16; short pcm = 1;
    fwrite(&fmt_size,     4, 1, f);
    fwrite(&pcm,          2, 1, f);
    fwrite(&num_ch,       2, 1, f);
    fwrite(&sample_rate,  4, 1, f);
    fwrite(&byte_rate,    4, 1, f);
    fwrite(&block_align,  2, 1, f);
    fwrite(&bits,         2, 1, f);
    fwrite("data",        1, 4, f);
    fwrite(&data_size,    4, 1, f);
    fwrite(samples, sizeof(short), num_samples, f);
    fclose(f);
}

/* encode() — encodes the given string of DTMF digits into a single WAV file.
   Uses the extern globals: output_filename, tone_duration_ms, gap_duration_ms. */
int encode(const char *digits) {
    int tone_samples = (int)((double)tone_duration_ms / 1000.0 * SAMPLE_RATE);
    int gap_samples  = (int)((double)gap_duration_ms  / 1000.0 * SAMPLE_RATE);

    /* Count valid digits to pre-allocate the full buffer */
    int valid = 0;
    for (const char *p = digits; *p; p++) {
        int low, high;
        if (dtmf_freqs(*p, &low, &high)) valid++;
    }
    if (valid == 0) { fputs("No valid DTMF digits found.\n", stderr); return 1; }

    /* Total samples: each digit gets tone_samples + gap_samples,
       minus one gap at the very end */
    int total_samples = valid * (tone_samples + gap_samples) - gap_samples;
    short *buf = calloc(total_samples, sizeof(short));
    if (!buf) { fputs("malloc failed\n", stderr); return 1; }

    int offset = 0;
    for (const char *p = digits; *p; p++) {
        int low, high;
        if (!dtmf_freqs(*p, &low, &high)) continue;

        dtmf_generate(buf + offset, tone_samples, low, high);
        printf("Encoded '%c'  offset=%-6d  (%d Hz + %d Hz)\n",
               *p, offset, low, high);
        offset += tone_samples;

        /* silence gap (buf is already zeroed by calloc) */
        if (offset < total_samples)
            offset += gap_samples;
    }

    write_wav(output_filename, buf, total_samples);
    printf("Written %d samples -> %s\n", total_samples, output_filename);
    free(buf);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/main.h"
#include "../include/decoder.h"

#define SAMPLE_RATE       8000
#define PI                3.14159265358979323846

/* All 8 DTMF frequencies */
static const int DTMF_FREQS[8] = {697, 770, 852, 941, 1209, 1336, 1477, 1633};

/* Map (row_index, col_index) -> character */
static const char DTMF_MAP[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'E', '0', 'F', 'D'}
};

/* ------------------------------------------------------------------ */
/* WAV loading                                                         */
/* ------------------------------------------------------------------ */

typedef struct {
    short  *samples;
    int     num_samples;
    int     sample_rate;
    short   num_channels;
    short   bits_per_sample;
} WavData;

static int load_wav(const char *filename, WavData *out) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return 0;
    }

    char riff[4], wave[4];
    int  chunk_size;
    fread(riff,       1, 4, f);
    fread(&chunk_size,4, 1, f);
    fread(wave,       1, 4, f);

    if (memcmp(riff, "RIFF", 4) || memcmp(wave, "WAVE", 4)) {
        fputs("decoder: not a RIFF/WAVE file\n", stderr);
        fclose(f);
        return 0;
    }

    short num_channels = 0, bits_per_sample = 0, audio_format = 0;
    int   sample_rate  = 0;

    /* Walk chunks until we hit "fmt " and "data" */
    while (!feof(f)) {
        char   id[4];
        int    size;
        if (fread(id,   1, 4, f) != 4) break;
        if (fread(&size,4, 1, f) != 1) break;

        if (memcmp(id, "fmt ", 4) == 0) {
            fread(&audio_format,   2, 1, f);
            fread(&num_channels,   2, 1, f);
            fread(&sample_rate,    4, 1, f);
            int byte_rate;   fread(&byte_rate,   4, 1, f);
            short block_align; fread(&block_align, 2, 1, f);
            fread(&bits_per_sample, 2, 1, f);
            /* skip any extra fmt bytes */
            if (size > 16) fseek(f, size - 16, SEEK_CUR);

        } else if (memcmp(id, "data", 4) == 0) {
            if (bits_per_sample != 16) {
                fputs("decoder: only 16-bit PCM supported\n", stderr);
                fclose(f);
                return 0;
            }
            int n = size / sizeof(short);
            short *samples = malloc(size);
            if (!samples) {
                fputs("decoder: malloc failed\n", stderr);
                fclose(f);
                return 0;
            }
            if ((int)fread(samples, sizeof(short), n, f) != n) {
                fputs("decoder: short read on data chunk\n", stderr);
                free(samples);
                fclose(f);
                return 0;
            }

            /* If stereo, mix down to mono */
            if (num_channels == 2) {
                int mono_n = n / 2;
                short *mono = malloc(mono_n * sizeof(short));
                if (!mono) { free(samples); fclose(f); return 0; }
                for (int i = 0; i < mono_n; i++)
                    mono[i] = (short)(((int)samples[i*2] + samples[i*2+1]) / 2);
                free(samples);
                samples = mono;
                n = mono_n;
            }

            out->samples        = samples;
            out->num_samples    = n;
            out->sample_rate    = sample_rate;
            out->num_channels   = 1;
            out->bits_per_sample = bits_per_sample;
            fclose(f);
            return 1;

        } else {
            /* Unknown chunk — skip */
            fseek(f, size, SEEK_CUR);
        }
    }

    fputs("decoder: data chunk not found\n", stderr);
    fclose(f);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Goertzel algorithm                                                  */
/* ------------------------------------------------------------------ */

/*
 * Returns the power (squared magnitude) of frequency `target_freq` in
 * `samples[0..n-1]` at `sample_rate`.
 */
static double goertzel(const short *samples, int n, int sample_rate,
                       int target_freq) {
    double  k     = (double)target_freq * n / sample_rate;
    double  omega = 2.0 * PI * k / n;
    double  coeff = 2.0 * cos(omega);
    double  s0 = 0.0, s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < n; i++) {
        s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    return s2 * s2 + s1 * s1 - coeff * s1 * s2;
}

/* ------------------------------------------------------------------ */
/* Energy in a window                                                  */
/* ------------------------------------------------------------------ */

static double window_energy(const short *samples, int n) {
    double e = 0.0;
    for (int i = 0; i < n; i++)
        e += (double)samples[i] * samples[i];
    return e / n;
}

/* ------------------------------------------------------------------ */
/* Identify the DTMF digit for a window                               */
/* ------------------------------------------------------------------ */

/*
 * Returns the recognised character, or '\0' if no valid DTMF pair found.
 * Writes the winning low/high indices into *row_out / *col_out.
 */
static char identify_digit(const short *samples, int n, int sample_rate) {
    double power[8];
    for (int i = 0; i < 8; i++)
        power[i] = goertzel(samples, n, sample_rate, DTMF_FREQS[i]);

    /* Find best low (0..3) and best high (4..7) */
    int    best_low = 0, best_high = 4;
    double max_low  = power[0], max_high = power[4];
    for (int i = 1; i < 4; i++)
        if (power[i] > max_low)  { max_low  = power[i]; best_low  = i; }
    for (int i = 5; i < 8; i++)
        if (power[i] > max_high) { max_high = power[i]; best_high = i; }

    /* Require the two winning frequencies to dominate */
    double total = 0.0;
    for (int i = 0; i < 8; i++) total += power[i];
    if (total == 0.0) return '\0';

    double ratio_low  = max_low  / total;
    double ratio_high = max_high / total;
    if (ratio_low < 0.1 || ratio_high < 0.1)
        return '\0';

    return DTMF_MAP[best_low][best_high - 4];
}

/* ------------------------------------------------------------------ */
/* Auto-detect tone duration from the WAV                             */
/* ------------------------------------------------------------------ */

/*
 * Scans through the file in small analysis windows, classifies each as
 * "active" (energy above threshold) or "silent", then measures the
 * median run-length of active regions → tone_ms, and silent → gap_ms.
 *
 * Returns 1 on success, 0 if detection failed.
 */
#define PROBE_WINDOW_MS  5      /* resolution of energy scan (ms)   */
#define MIN_RUNS         2      /* need at least 2 tones to measure */

static int autodetect_durations(const WavData *wav,
                                int *out_tone_ms, int *out_gap_ms) {
    int probe_samples = (int)((double)PROBE_WINDOW_MS / 1000.0 * wav->sample_rate);
    if (probe_samples < 1) probe_samples = 1;

    int num_windows = wav->num_samples / probe_samples;
    if (num_windows < 2) return 0;

    /* --- compute per-window energy --- */
    double *energy = malloc(num_windows * sizeof(double));
    if (!energy) return 0;
    for (int w = 0; w < num_windows; w++)
        energy[w] = window_energy(wav->samples + w * probe_samples,
                                  probe_samples);

    /* --- pick threshold as a fraction of peak energy --- */
    double peak = 0.0;
    for (int w = 0; w < num_windows; w++)
        if (energy[w] > peak) peak = energy[w];

    double threshold = peak * 0.10;

    /* --- label each window: 1=active, 0=silent --- */
    char *active = malloc(num_windows);
    if (!active) { free(energy); return 0; }
    for (int w = 0; w < num_windows; w++)
        active[w] = energy[w] > threshold ? 1 : 0;

    /* --- collect run lengths for active and silent regions --- */
    int  *active_runs = malloc(num_windows * sizeof(int));
    int  *silent_runs = malloc(num_windows * sizeof(int));
    int   n_active = 0, n_silent = 0;

    if (!active_runs || !silent_runs) {
        free(energy); free(active);
        free(active_runs); free(silent_runs);
        return 0;
    }

    int w = 0;
    while (w < num_windows) {
        char state = active[w];
        int  start = w;
        while (w < num_windows && active[w] == state) w++;
        int run_len = w - start;   /* in probe-windows */
        if (state) active_runs[n_active++] = run_len;
        else        silent_runs[n_silent++] = run_len;
    }

    free(energy);
    free(active);

    if (n_active < MIN_RUNS) {
        free(active_runs); free(silent_runs);
        return 0;
    }

    /*
     * Ignore the first and last silent runs (they may be leading/trailing
     * silence, not inter-tone gaps).
     */
    int gap_start = (n_silent > 0 && silent_runs[0] > active_runs[0]) ? 1 : 0;
    int gap_end   = n_silent;
    int usable_gaps = gap_end - gap_start;
    if (n_silent > 1) gap_end--;   /* drop trailing silence */

    /* Median of active runs → tone duration */
    /* Simple insertion sort on small arrays is fine */
    for (int i = 1; i < n_active; i++) {
        int key = active_runs[i], j = i - 1;
        while (j >= 0 && active_runs[j] > key) { active_runs[j+1] = active_runs[j]; j--; }
        active_runs[j+1] = key;
    }
    double median_tone_windows = active_runs[n_active / 2];

    double median_gap_windows = PROBE_WINDOW_MS; /* default: 1 probe window */
    if (usable_gaps > 0) {
        int *usable = silent_runs + gap_start;
        int  ug     = gap_end - gap_start;
        if (ug > 0) {
            for (int i = 1; i < ug; i++) {
                int key = usable[i], j = i - 1;
                while (j >= 0 && usable[j] > key) { usable[j+1] = usable[j]; j--; }
                usable[j+1] = key;
            }
            median_gap_windows = usable[ug / 2];
        }
    }

    free(active_runs);
    free(silent_runs);

    *out_tone_ms = (int)(median_tone_windows * PROBE_WINDOW_MS + 0.5);
    *out_gap_ms  = (int)(median_gap_windows  * PROBE_WINDOW_MS + 0.5);

    /* Clamp to sane values */
    if (*out_tone_ms < 1)  *out_tone_ms = 1;
    if (*out_gap_ms  < 1)  *out_gap_ms  = 1;

    return 1;
}

/* ------------------------------------------------------------------ */
/* Main decode routine                                                 */
/* ------------------------------------------------------------------ */

int decoder(void) {
    if (!(flags & DECODE_FLAG)) {
        fputs("decoder: DECODE_FLAG not set\n", stderr);
        return 1;
    }
    if (!(flags & INPUT_FILENAME_FLAG) || !input_filename) {
        fputs("decoder: no input file specified (-i <file>)\n", stderr);
        return 1;
    }

    /* 1. Load WAV */
    WavData wav;
    if (!load_wav(input_filename, &wav)) return 1;

    LOG("decoder: loaded %d samples @ %d Hz from %s\n",
        wav.num_samples, wav.sample_rate, input_filename);

    /* 2. Auto-detect tone/gap durations unless overridden by the user */
    int detected_tone_ms = tone_duration_ms;
    int detected_gap_ms  = gap_duration_ms;

    if (!(flags & TONE_DURATION_FLAG) || !(flags & GAP_DURATION_FLAG)) {
        int dt = 0, dg = 0;
        if (autodetect_durations(&wav, &dt, &dg)) {
            if (!(flags & TONE_DURATION_FLAG)) detected_tone_ms = dt;
            if (!(flags & GAP_DURATION_FLAG))  detected_gap_ms  = dg;
            LOG("decoder: auto-detected tone=%d ms  gap=%d ms\n", dt, dg);
        } else {
            LOG("decoder: auto-detection failed, using defaults "
                "(tone=%d ms, gap=%d ms)\n",
                detected_tone_ms, detected_gap_ms);
        }
    }

    /* 3. Derive analysis window = tone duration */
    int tone_samples = (int)((double)detected_tone_ms / 1000.0 * wav.sample_rate);
    int gap_samples  = (int)((double)detected_gap_ms  / 1000.0 * wav.sample_rate);
    int step         = tone_samples + gap_samples;

    if (tone_samples < 8) {
        fputs("decoder: tone window too short for Goertzel analysis\n", stderr);
        free(wav.samples);
        return 1;
    }

    LOG("decoder: tone_samples=%d  gap_samples=%d  step=%d\n",
        tone_samples, gap_samples, step);

    /* 4. Compute energy threshold from peak energy in first tone window */
    double peak_energy = 0.0;
    for (int offset = 0; offset + tone_samples <= wav.num_samples;
         offset += step) {
        double e = window_energy(wav.samples + offset, tone_samples);
        if (e > peak_energy) peak_energy = e;
    }
    double energy_threshold = peak_energy * 0.10;

    /* 5. Decode each window */
    int    result_cap  = 256;
    int    result_len  = 0;
    char  *result      = malloc(result_cap);
    if (!result) { free(wav.samples); return 1; }

    for (int offset = 0; offset + tone_samples <= wav.num_samples;
         offset += step) {

        double e = window_energy(wav.samples + offset, tone_samples);
        if (e < energy_threshold) continue;   /* silence window, skip */

        char ch = identify_digit(wav.samples + offset, tone_samples,
                                 wav.sample_rate);
        if (ch == '\0') continue;

        LOG("decoder: offset=%-6d  energy=%.0f  -> '%c'\n", offset, e, ch);

        if (result_len + 1 >= result_cap) {
            result_cap *= 2;
            char *tmp = realloc(result, result_cap);
            if (!tmp) { free(result); free(wav.samples); return 1; }
            result = tmp;
        }
        result[result_len++] = ch;
    }
    result[result_len] = '\0';

    /* 6. Apply reverse compat mapping if needed */
    if (flags & COMPAT_FLAG) {
        for (char *p = result; *p; p++) {
            if (*p == 'E') *p = '*';
            else if (*p == 'F') *p = '#';
        }
    }

    /* 7. Output */
    if (flags & OUTPUT_FILENAME_FLAG && output_filename) {
        FILE *out = fopen(output_filename, "w");
        if (!out) {
            perror("fopen output");
            free(result); free(wav.samples);
            return 1;
        }
        fputs(result, out);
        fputc('\n', out);
        fclose(out);
        LOG("decoder: wrote result to %s\n", output_filename);
    } else {
        puts(result);
    }

    free(result);
    free(wav.samples);
    return 0;
}

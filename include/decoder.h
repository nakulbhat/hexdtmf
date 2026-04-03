#ifndef DECODER_H
#define DECODER_H

/*
 * decoder()
 *
 * Reads a WAV file specified by the global `input_filename`, auto-detects
 * tone and gap durations (unless -t / -g are provided on the command line),
 * and prints the decoded DTMF digit string to stdout or to `output_filename`
 * if -o was given.
 *
 * Returns 0 on success, 1 on failure.
 *
 * Relies on globals from main.h:
 *   flags, input_filename, output_filename, tone_duration_ms, gap_duration_ms
 */
int decoder(void);

#endif /* DECODER_H */

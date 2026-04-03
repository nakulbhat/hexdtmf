#ifndef DECODER_H
#define DECODER_H

/**
 * Reads a WAV file and performs Goertzel analysis to extract DTMF digits.
 * Returns 0 on success, 1 on failure.
 */
int decoder(void);

#endif /* DECODER_H */

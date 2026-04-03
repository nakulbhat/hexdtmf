# `hexdtmf` - Hexadecimal DTMF Encoder/Decoder

A small command-line toy program for encoding and decoding DTMF tones (telephone keypad sounds) using hexadecimal-style digits.

This program can:

* **Encode** digits into a `.wav` file containing DTMF tones
* **Decode** DTMF tones from a `.wav` file back into digits

---

# Install

Requirements:

* GCC
* Make
* POSIX system (Linux/macOS recommended)

Build:

```bash
make
```

This will produce:

```
hexdtmf
```

Clean build files:

```bash
make clean
```

---

# Usage

## Encode

Encode a digit string into a WAV file:

```bash
./hexdtmf encode 123ABC
```

Output defaults to:

```
output.wav
```

Specify output file:

```bash
./hexdtmf encode 123ABC -o tones.wav
```

Read digits from file:

```bash
./hexdtmf encode -i digits.txt -o tones.wav
```

Read digits from stdin:

```bash
echo "123ABC" | ./hexdtmf encode
```

---

## Decode

Decode a WAV file:

```bash
./hexdtmf decode -i tones.wav
```

Write output to file:

```bash
./hexdtmf decode -i tones.wav -o output.txt
```

Decode from stdin:
```bash
cat tones.wav | ./hexdtmf decode
```

---

## Options

```
-h, --help        Show help
-v, --verbose     Verbose output
-t <ms>           Tone duration (milliseconds)
-g <ms>           Gap duration (milliseconds)
-o <file>         Output file
-i <file>         Input file
-c, --compat      Use * and # instead of E and F
```

---

## Supported Digits

```
0–9
A–D
E (maps to *)
F (maps to #)
```

Use `--compat` to work with `*` and `#` directly.

---

# Examples

```bash
./hexdtmf encode 1800DEADBEEF -o phone.wav
./hexdtmf decode -i phone.wav
```

---

# Disclaimer

This is a toy program and not under active development. If you want
to use a real DTMF encoder and decoder, see [dtmflib](https://github.com/gbonacini/dtmflib).

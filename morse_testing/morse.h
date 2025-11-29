#ifndef MORSE_H
#define MORSE_H

void morse_send_string(const char* text);

// Alternativ 2: generera signaler i buffer:
typedef struct {
    int frequency;   // Hz, 0 = tystnad
    int duration_ms; // längd
} MorseSignal;

int morse_generate(const char* text, MorseSignal* out, int max_len);

#endif

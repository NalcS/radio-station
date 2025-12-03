#include "morse.h"


// --- Morse tabell ---
typedef struct { char c; const char* morse; } MorseEntry;
static const MorseEntry morse_table[] = {
    {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
    {'E', "."}, {'F', "..-."}, {'G', "--."}, {'H', "...."},
    {'I', ".."}, {'J', ".---"}, {'K', "-.-"}, {'L', ".-.."},
    {'M', "--"}, {'N', "-."}, {'O', "---"}, {'P', ".--."},
    {'Q', "--.-"}, {'R', ".-."}, {'S', "..."}, {'T', "-"},
    {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"},
    {'Y', "-.--"}, {'Z', "--.."},
    {'1', ".----"},{'2', "..---"},{'3', "...--"},{'4', "....-"},
    {'5', "....."},{'6', "-...."},{'7', "--..."},{'8', "---.."},
    {'9', "----."},{'0', "-----"},
    {' ', " "}  // ord-mellanrum
};
static const int morse_table_len = sizeof(morse_table)/sizeof(MorseEntry);

#define MORSE_UNIT_MS 120

// --- Lokal delay för morse ---
static void morse_delay_ms(int ms) {
    // 1 ms ≈ 40000 iterationer i er clock/loop
    for (volatile int i = 0; i < ms * 40000; i++);
}

// --- Generera ca 600 Hz bärvåg på GDO0 ---
static void morse_tone(int freq_hz, int duration_ms) {
    if (freq_hz <= 0) {
        // Tystnad
        set_data(GDO0_PIN, 0);
        morse_delay_ms(duration_ms);
        return;
    }

    // Enkel kvadratvågsgenerator
    int cycles = duration_ms * freq_hz;
    int half_period_delay = (1000 / freq_hz) / 2; // ms per halva

    for (int i = 0; i < cycles; i++) {
        set_data(GDO0_PIN, 1);
        morse_delay_ms(half_period_delay);

        set_data(GDO0_PIN, 0);
        morse_delay_ms(half_period_delay);
    }
}

static const char* lookup_morse(char c) {
    if (c >= 'a' && c <= 'z') c -= 32; // uppercase
    for (int i = 0; i < morse_table_len; i++) {
        if (morse_table[i].c == c)
            return morse_table[i].morse;
    }
    return "";  // okänd → tyst
}

void morse_send_string(const char* text) {
    for (const char* p = text; *p; p++) {

        const char* code = lookup_morse(*p);

        if (code[0] == '\0') {
            // okänd symbol
            morse_tone(0, MORSE_UNIT_MS * 3);
            continue;
        }
        if (code[0] == ' ') {
            // ordmellanrum
            morse_tone(0, MORSE_UNIT_MS * 7);
            continue;
        }

        // varje tecken
        for (const char* s = code; *s; s++) {
            if (*s == '.') {
                morse_tone(600, MORSE_UNIT_MS);           // dot
            } else if (*s == '-') {
                morse_tone(600, MORSE_UNIT_MS * 3);       // dash
            }
            morse_tone(0, MORSE_UNIT_MS);                 // mellan element
        }

        // mellan bokstäver: totalt 3 enheter → 1 enhet har redan körts
        morse_tone(0, MORSE_UNIT_MS * 3);
    }
}


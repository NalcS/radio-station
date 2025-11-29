#include "morse.h"

// --- Morse helpers ---
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
    {' ', " "} // space as word gap
};
static const int morse_table_len = sizeof(morse_table)/sizeof(morse_table[0]);

const char* lookup_morse(char ch) {
    if (ch >= 'a' && ch <= 'z') ch -= 32; // uppercase
    for (int i = 0; i < morse_table_len; i++) {
        if (morse_table[i].c == ch) return morse_table[i].morse;
    }
    return ""; // unknown -> silence
}

// unit in ms for dot
#define MORSE_UNIT_MS 120

void send_morse_string(const char* s) {
    for (const char* p = s; *p; p++) {
        const char* code = lookup_morse(*p);
        if (code[0] == '\0') {
            // unknown - short pause
            play_tone(0, MORSE_UNIT_MS * 3);
            continue;
        }
        if (code[0] == ' ') {
            // word gap
            play_tone(0, MORSE_UNIT_MS * 7);
            continue;
        }
        // for each symbol in code
        for (const char* sym = code; *sym; sym++) {
            if (*sym == '.') {
                play_tone(600, MORSE_UNIT_MS); // dot: 1 unit
            } else if (*sym == '-') {
                play_tone(600, MORSE_UNIT_MS * 3); // dash: 3 units
            }
            // gap between elements
            play_tone(0, MORSE_UNIT_MS);
        }
        // gap between letters
        play_tone(0, MORSE_UNIT_MS * 2); // total between letters = 3 units
    }
}

int morse_generate(const char* s, MorseSignal* out, int max_len) {
    int n = 0;

    for (const char* p = s; *p && n < max_len; p++) {
        const char* code = lookup_morse(*p);

        if (code[0] == '\0') {
            out[n++] = (MorseSignal){0, MORSE_UNIT_MS * 3};
            continue;
        }

        if (code[0] == ' ') {
            out[n++] = (MorseSignal){0, MORSE_UNIT_MS * 7};
            continue;
        }

        for (const char* sym = code; *sym && n < max_len; sym++) {
            if (*sym == '.') {
                out[n++] = (MorseSignal){600, MORSE_UNIT_MS};
            } else if (*sym == '-') {
                out[n++] = (MorseSignal){600, MORSE_UNIT_MS * 3};
            }
            if (n < max_len) {
                out[n++] = (MorseSignal){0, MORSE_UNIT_MS};
            }
        }
        if (n < max_len) {
            out[n++] = (MorseSignal){0, MORSE_UNIT_MS * 2};
        }
    }

    return n; // antal element
}





// TODO: fixa meny, moduläritet i main programmet.

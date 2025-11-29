// Hardware addresses for LEDs and switches (From Lab 3)
#define LED_ADDR       0x04000000
#define SWITCH_ADDR    0x04000010

volatile unsigned int* leds     = (unsigned int*)LED_ADDR;
volatile unsigned int* switches = (unsigned int*)SWITCH_ADDR;

// Read the lower N bits of switch register
unsigned int read_switch_bits(unsigned int mask) {
    return (*switches) & mask;
}

int main() {

    // Init CC1101 / GPIO SPI
    radio_init();

    while (1) {
        // läs två lägsta bitarna från switcharna för att välja mellan 4 låtar
        int selected_song = read_switch_bits(0x3);

        // Uppdatera LED-lamporna för att visa vald låt
        *leds = selected_song;

        // Spela låten
        play_song(selected_song);
    }

    return 0;
}

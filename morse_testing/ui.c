#include "ui.h"

#define LED_ADDR       0x04000000
#define SWITCH_ADDR    0x04000010

volatile unsigned int* leds     = (unsigned int*)LED_ADDR;
volatile unsigned int* switches = (unsigned int*)SWITCH_ADDR;

// Read 3 least significant bits (0–2) from switches
unsigned int ui_get_selected_file() {
    return (*switches) & 0x7;   // mask = 0b00000111
}

// Update LEDs (show 0–7)
void ui_update_leds(unsigned int value) {
    *leds = value & 0x7;
}



// Example usage:
//while (1)
//{
    // If there are files available, check selected file
//  if (file_amount > 0)
//  {
        // Read switches (0–7)
//      int selected = ui_get_selected_file();

        // Update LEDs
//      ui_update_leds(selected);

        // Check if selected index exists
        // selected = 0 --> file #1
//      if (selected < file_amount)
//      {
//      transmit_current_file(selected + 1);   // Account for 1-based indexing
//      }
//      If selected >= file_amount → do nothing (no such file)
//  }

    // Check for new file uploads
//  if (is_new_file())
//  {  
//      file_amount++;
//      transfer_new_file(file_amount);
//  }
//}

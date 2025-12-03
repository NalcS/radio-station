extern void print(const char*);
extern void print_dec(const unsigned int);
extern void print_hex32(const unsigned int);



#define GPIO_BASE_ADDRESS2 0x040000e0



#define SCL_PIN     (1 << 0)
#define CSN_PIN     (1 << 1)
#define MOSI_PIN    (1 << 4)
#define MISO_PIN    (1 << 6)
#define GDO0_PIN    (1 << 5)


volatile unsigned int* data = (unsigned int*)(GPIO_BASE_ADDRESS2 + 0 * 0x4);
volatile unsigned int* direction = (unsigned int*)(GPIO_BASE_ADDRESS2 + 1 * 0x4);



#define CC1101_SRES 0x30        
#define CC1101_SFSTXON 0x31     
#define CC1101_SXOFF 0x32       
#define CC1101_SCAL 0x33        
#define CC1101_SRX 0x34         
#define CC1101_STX 0x35         
#define CC1101_SIDLE 0x36       
#define CC1101_SWOR 0x38        
#define CC1101_SPWD 0x39        
#define CC1101_SFRX 0x3A        
#define CC1101_SFTX 0x3B        
#define CC1101_SWORRST 0x3C     
#define CC1101_SNOP 0x3D        


#define CC1101_FREQ2 0x0D       
#define CC1101_FREQ1 0x0E       
#define CC1101_FREQ0 0x0F       
#define CC1101_MDMCFG2 0x12     
#define CC1101_PKTCTRL0 0x08    
#define CC1101_IOCFG0 0x02      

#define CC1101_FSCTRL1    0x0B
#define CC1101_FSCTRL0    0x0C
#define CC1101_MCSM0      0x18
#define CC1101_DEVIATN    0x15
#define CC1101_FREND0     0x22


#define CC1101_PATABLE 0x3E     



#define FILE_ALLOC_SIZE 0x00500000

#define FILE_UPLOAD_BASE_ADDRESS 0x00100000
#define FILE_UPLOAD_BASE_ADDRESS_NON_CACHE 0xA0100000


void delay(int amount) { 
    for (volatile int i = 0; i < amount; i++)
    {
        
    }
    
    
}

void set_direction(int pins, unsigned char setToTrue){
    if (setToTrue)
    {
        *direction |= pins;
    }
    else 
    {
        *direction &= ~pins;
    }
}
void set_data(int pins, unsigned char setToTrue){
    if (setToTrue)
    {
        *data |= pins;
    }
    else 
    {
        *data &= ~pins;
    }
}
unsigned int get_data(int pins) {
    return ((*data) & pins);
}

void spi_init(){
    set_direction(SCL_PIN | CSN_PIN | MOSI_PIN | GDO0_PIN, 1);
    set_direction(MISO_PIN, 0);
    
    set_data(CSN_PIN, 1);
    set_data(SCL_PIN, 0);

}

unsigned char spi_transfer(unsigned char send_byte)
{

    unsigned char received_byte;

    for (int i = 0; i < 8; i++) 
    {
        if (send_byte>>7) 
        {
            set_data(MOSI_PIN, 1);
        }
        else {
            set_data(MOSI_PIN, 0);
        }
        send_byte = send_byte << 1; 


        set_data(SCL_PIN, 1);

        delay(10);

        if (get_data(MISO_PIN) == MISO_PIN) 
        {
            received_byte |= 0b1;
        }
        else {
            received_byte |= 0b0;
        }
        received_byte << 1;

        set_data(SCL_PIN, 0);

        delay(10);
        
    }

    return received_byte;
}

void send_strobe(unsigned char strobe){
    set_data(CSN_PIN, 0);
    spi_transfer(strobe);
    set_data(CSN_PIN, 1);
}

void reset_CC1101(){ 
    set_data(CSN_PIN, 0);
    delay(10);
    set_data(CSN_PIN, 1);
    delay(10);
    send_strobe(CC1101_SRES);
    delay(1000);

}

void write_reg_CC1101(unsigned char address, unsigned char content) {

    set_data(CSN_PIN, 0);

    spi_transfer(address);
    spi_transfer(content);

    set_data(CSN_PIN, 1);
    
}

void print_bi(unsigned char byte) {
    for (int i = 0; i < 8; i++)
    {   
        if (byte>>7 == 0b1)
        {
            print("1");
        }
        else{
            print("0");
        }
        byte = byte << 1;
    }
    print("\n");
}

unsigned char is_new_file(){
    volatile unsigned char *file_data = (unsigned char*)FILE_UPLOAD_BASE_ADDRESS_NON_CACHE;

    if (file_data[0] == 0b11111111)
    {
        return 0;
    }
    else {
        return 1; //new file
    }
}

void transfer_new_file(volatile unsigned char file_amount){
    
    delay(10000); //wait for header info to be uploaded

    
    
    volatile unsigned char *file_data = (unsigned char*)FILE_UPLOAD_BASE_ADDRESS;


    int data_byte_amount = *(unsigned int*)(&file_data[40]);
    print_dec(data_byte_amount);
    delay(data_byte_amount*10); //wait appropriate amount

    
    for (int i = 0; i < data_byte_amount+44; i++)
    {
        file_data[file_amount * FILE_ALLOC_SIZE + i] = file_data[i];
    }


    file_data[0] = 0b11111111;
}



void transmit_current_file(int current_file) {
    int base_file_address = 0x00100000;

    print("Transmitting file\n");

    //get header info
    volatile unsigned char *file_data = (unsigned char*)FILE_UPLOAD_BASE_ADDRESS;
    int data_byte_amount = *(unsigned int*)(&file_data[current_file * FILE_ALLOC_SIZE + 40]);
    print_dec(data_byte_amount);
    
    unsigned int accumulator = 0;
    unsigned char sample = 128; //start with silence
    
    int loops_per_sample = 110; 

    print("Transmitting audio:\n");

    
    for (int i = 44; i < data_byte_amount+44; i++)
    {            
        sample = file_data[current_file * FILE_ALLOC_SIZE + i];

        for (int k = 0; k < loops_per_sample; k++) {
            
            accumulator += sample;

            if (accumulator >= 255) {
                set_data(GDO0_PIN, 1); //output high freq
                accumulator -= 255;
            } else {
                set_data(GDO0_PIN, 0); //output low freq
            }
        }
    }
    
}

void config_CC1101() {

    write_reg_CC1101(CC1101_IOCFG0, 0x0D); 



    write_reg_CC1101(CC1101_PKTCTRL0, 0x32);

   

    write_reg_CC1101(CC1101_MDMCFG2, 0b01000000);
    

    write_reg_CC1101(CC1101_FREQ2, 0x10);

    write_reg_CC1101(CC1101_FREQ1, 0xA7);

    write_reg_CC1101(CC1101_FREQ0, 0x62);

    write_reg_CC1101(CC1101_DEVIATN, 0x47); 

    write_reg_CC1101(CC1101_MCSM0, 0x18);

    set_data(CSN_PIN, 0);
    spi_transfer(CC1101_PATABLE); 
    spi_transfer(0xC0);           
    set_data(CSN_PIN, 1);


    send_strobe(CC1101_SFTX);
    
    send_strobe(CC1101_STX);
}




void set_displays(int display_number, int value){
  int base_address = 0x04000050 + (display_number * 0x10);
  volatile int* display = (int*) base_address;
  switch (value)
  {
  case 0:
    *display = 0b1000000;
    break;
  case 1:
    *display = 0b1111001;
    break;
  case 2:
    *display = 0b0100100;
    break;
  case 3:
    *display = 0b0110000;
    break;
  case 4:
    *display = 0b0011001;
    break;
  case 5:
    *display = 0b0010010;
    break;
  case 6:
    *display = 0b0000010;
    break;
  case 7:
    *display = 0b1111000;
    break;
  case 8:
    *display = 0b0000000;
    break;
  case 9:
    *display = 0b0011000;
    break;
  case -1:
    *display = 0b1111111;
  
  default:
    break;
  }
}

int get_sw( void ) {
  int return_value = 0;
  volatile int* sw_address = (int*) 0x04000010;
  return_value = (*sw_address) & 0x3ff;

  return return_value;
}

int get_btn( void ) {
  int return_value = 0;
  volatile int* btn_address = (int*) 0x040000d0;
  return_value = (*btn_address) & 0b1;

  return return_value;
}


// MORSE HELPERS
unsigned char is_new_text() {
    volatile unsigned char* file_data = (unsigned char*)FILE_UPLOAD_BASE_ADDRESS_NON_CACHE;

    // text file starts with 0xFF if no new text
    if (file_data[0] == 0xFF)
        return 0;   // no new text file
    return 1;       // new text file
}



//MORSE TEMP COPY

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

#define MORSE_UNIT_MS 12

// --- Lokal delay för morse ---
static void morse_delay_ms(int ms) {
    // 1 ms ≈ 40000 iterationer i er clock/loop
    for (volatile int i = 0; i < ms * 40000; i++);
}

// --- Generera ca 600 Hz bärvåg på GDO0 ---
static void morse_tone1(int freq_hz, int duration_ms) {
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

    for (int i = 0; i < cycles; i++) {
        set_data(GDO0_PIN, 1);
        delay(3000);

        set_data(GDO0_PIN, 0);
        delay(3000);
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
                morse_tone(10, MORSE_UNIT_MS);           // dot
            } else if (*s == '-') {
                morse_tone(10, MORSE_UNIT_MS * 3);       // dash
            }
            morse_tone(0, MORSE_UNIT_MS);                 // mellan element
        }

        // mellan bokstäver: totalt 3 enheter → 1 enhet har redan körts
        morse_tone(0, MORSE_UNIT_MS * 3);
    }
}

//END




void transfer_new_text(int text_index) {

    volatile unsigned char* src = (unsigned char*)FILE_UPLOAD_BASE_ADDRESS;
    volatile unsigned char* dst = (unsigned char*)(FILE_UPLOAD_BASE_ADDRESS + text_index * FILE_ALLOC_SIZE);

    // Wait until the upload is complete 
    delay(500000); //this one is relativly short as the fiels are expected to be relativly short, as compared to the size of an audio file

    // Copy until we find a NULL byte
    int i = 0;
    while (src[i] != 0 && i < FILE_ALLOC_SIZE-1) {
        dst[i] = src[i];
        i++;
    }

    // Add NULL termination in destination
    dst[i] = 0;

    // Mark as done – set upload first byte to FE
    src[0] = 0xFF;
}


void transmit_text_file(int index) {
    volatile unsigned char* file_data =
        (unsigned char*)(FILE_UPLOAD_BASE_ADDRESS + index * FILE_ALLOC_SIZE);

    morse_send_string((const char*)file_data);
}




int main() {
    
    
    int file_amount = 0;
    int current_file = 0;
    int playing = 0;

    


    spi_init();

    reset_CC1101();

    config_CC1101();

    set_direction(GDO0_PIN, 1); 
    


    volatile unsigned char *file_data = (unsigned char*)FILE_UPLOAD_BASE_ADDRESS;
    file_data[0] = 0b11111111;
    
    
    set_displays(0, 0);
    set_displays(1, -1);
    set_displays(2, -1);
    set_displays(3, -1);
    set_displays(4, 0);
    set_displays(5, -1);

    while (1)
    {
        int sw = get_sw();
        int morse_mode = (sw & (1 << 9)) != 0;   // SW9 = Morse mode
        int file_select = sw & 0x1FF;         // SW0-8 = file select

        if (get_btn() && file_amount > 0 && current_file > 0) {
            if (!morse_mode) {
                transmit_current_file(current_file); // Wav
            } else {
                transmit_text_file(current_file);    // Morse
            }
        }

        if (is_new_file())
        {
            file_amount++;
            transfer_new_file(file_amount);
        }

        if (file_amount >= file_select) 
        {
            current_file = file_select;
            
        }

        set_displays(0, current_file);
        set_displays(4, file_amount);

        set_displays(1, -1);
        set_displays(2, -1);
        set_displays(3, -1);
        set_displays(5, -1);
        delay(10000);
    }
}
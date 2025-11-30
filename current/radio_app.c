extern void print(const char*);
extern void print_dec(const unsigned int);
extern void print_hex32(const unsigned int);

//0x040000e0-0x040000ef (Pins 0-31)
//0x040000f0-0x040000ff (Pins 32-35)

#define GPIO_BASE_ADDRESS2 0x040000e0
//we will only use the 0-31 pins so we can disregard the other address

//  PINS:
//  BLACK   GROUND                  PIN 30 (GND)
//  RED     VCC (POWER)             PIN 29 (3.3 V)
//  BROWN   SCL (CLOCK)             PIN 1 ([0])
//  GREEN   CSN (CHIP SELECT)       PIN 2 ([1])
//  YELLOW  MOSI (MASTER OUTPUT)    PIN 5 ([4])
//  ORANGE  MISO (MASTER INPUT)     PIN 7 ([6])
//  WHITE   GPO0                    PIN 6 ([5]) (technically information output pin but we will use for async input)


#define SCL_PIN     (1 << 0)
#define CSN_PIN     (1 << 1)
#define MOSI_PIN    (1 << 4)
#define MISO_PIN    (1 << 6)
#define GDO0_PIN    (1 << 5)

//GPIO PINS ADDRESSES
volatile unsigned int* data = (unsigned int*)(GPIO_BASE_ADDRESS2 + 0 * 0x4);
volatile unsigned int* direction = (unsigned int*)(GPIO_BASE_ADDRESS2 + 1 * 0x4);


//CC1101 STUFF:
//  COMMAND STROBES
//      source: https://www.ti.com/lit/ds/symlink/cc1101.pdf
#define CC1101_SRES 0x30        // Reset chip.
#define CC1101_SFSTXON 0x31     // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX (with CCA): Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
#define CC1101_SXOFF 0x32       // Turn off crystal oscillator.
#define CC1101_SCAL 0x33        // Calibrate frequency synthesizer and turn it off. SCAL can be strobed from IDLE mode without setting manual calibration mode (MCSM0.FS_AUTOCAL=0)
#define CC1101_SRX 0x34         // Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1.
#define CC1101_STX 0x35         // In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled: Only go to TX if channel is clear.
#define CC1101_SIDLE 0x36       // Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable.
#define CC1101_SWOR 0x38        // Start automatic RX polling sequence (Wake-on-Radio) as described in Section 19.5 if WORCTRL.RC_PD=0.
#define CC1101_SPWD 0x39        // Enter power down mode when CSn goes high.
#define CC1101_SFRX 0x3A        // Flush the RX FIFO buffer. Only issue SFRX in IDLE or RXFIFO_OVERFLOW states.
#define CC1101_SFTX 0x3B        // Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.
#define CC1101_SWORRST 0x3C     // Reset real time clock to Event1 value.
#define CC1101_SNOP 0x3D        // No operation. May be used to get access to the chip status byte.

//  CONFIGURATION REGISTERS
#define CC1101_FREQ2 0x0D       // Frequency Control Word, High Byte
#define CC1101_FREQ1 0x0E       // Frequency Control Word, Middle Byte
#define CC1101_FREQ0 0x0F       // Frequency Control Word, Low Byte
#define CC1101_MDMCFG2 0x12     // Modem Configuration
#define CC1101_PKTCTRL0 0x08    //Packet automation control 
#define CC1101_IOCFG0 0x02      // GDO0 Output Pin Configuration

#define CC1101_FSCTRL1    0x0B
#define CC1101_FSCTRL0    0x0C
#define CC1101_MCSM0      0x18
#define CC1101_DEVIATN    0x15
#define CC1101_FREND0     0x22

//  something else that we use for setting the power of the transmission
#define CC1101_PATABLE 0x3E     // PA Power Setting Table


//memory that gets allocated for each audio file
#define FILE_ALLOC_SIZE 0x00500000
//address where you upload files
#define FILE_UPLOAD_BASE_ADDRESS 0x00100000
#define FILE_UPLOAD_BASE_ADDRESS_NON_CACHE 0xA0100000


void delay(int amount) { //volatile prevents optimising away loop
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
    //config GPIO pins
    set_direction(SCL_PIN | CSN_PIN | MOSI_PIN | GDO0_PIN, 1);
    set_direction(MISO_PIN, 0);
    
    //Set chip select high
    set_data(CSN_PIN, 1);
    //set clock to low
    set_data(SCL_PIN, 0);

}

unsigned char spi_transfer(unsigned char send_byte)
{

    unsigned char received_byte;

    //send each bit in the byte (and read each from MISO)
    for (int i = 0; i < 8; i++) //consider a more suitable datatype since we know we will only go up to 8?
    {
        //write data
        if (send_byte>>7) //check if the most significant bit is 1
        {
            set_data(MOSI_PIN, 1);
        }
        else {
            set_data(MOSI_PIN, 0);
        }
        send_byte = send_byte << 1; 


        //set clock high
        set_data(SCL_PIN, 1);

        delay(10);

        //read input
        if (get_data(MISO_PIN) == MISO_PIN) //if slave is sending a 1
        {
            received_byte |= 0b1;
        }
        else {
            received_byte |= 0b0;
        }
        received_byte << 1;

        //set clock low
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
    //tell slave to listen      (crazy comment vro) 
    set_data(CSN_PIN, 0);
    delay(10);
    set_data(CSN_PIN, 1);
    delay(10);
    //send reset command strobe
    send_strobe(CC1101_SRES);
    delay(1000);

}

void write_reg_CC1101(unsigned char address, unsigned char content) {
    //chip select low
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
    //legit such bullshit i spent so long to find that the cache is wrong and you have to specify that it shouldn't read the cache
    //  if you don't use the NON_CACHE one it won't work because the cache is not updated and doesn't update when we upload the file
    //      might be something to write about in the report

    if (file_data[0] == 0b11111111)
    {
        return 0;
    }
    else {
        return 1; //new file
    }
}

void transfer_new_file(volatile unsigned char file_amount){
    
    //wait until file has finished uploading
    delay(5000000);
    //not optimal since we cannot garantee that this is the actual amount of time the upload took
    //TODO: make it based on the data_byte_amount

    //get header info
    volatile unsigned char *file_data = (unsigned char*)FILE_UPLOAD_BASE_ADDRESS;


    int data_byte_amount = *(unsigned int*)(&file_data[40]);
    print_dec(data_byte_amount);


    //first we will copy over the file from the upload section
    //  the upload section is 0x00100000-0x00600000
    //      meaning that the file size limit is 5mb
    for (int i = 0; i < data_byte_amount+44; i++)
    {
        //copy the value of the uploaded file to its allocated location
        file_data[file_amount * FILE_ALLOC_SIZE + i] = file_data[i];
    }


    //clear the first byte
    //  setting the first byte to 0b11111111 so that we can detect when it changes
    //      i chose 0b11111111 because a wav file should never start with that binary sequence
    file_data[0] = 0b11111111;
}


void transmit_current_file(int current_file) {
    int base_file_address = 0x00100000;

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
    //sets GDO0 as "Serial Data Output"
    //makes it become the input for the slave
    write_reg_CC1101(CC1101_IOCFG0, 0x0D); 


    //sets to format=3 (Asynchronous Serial Mode)
    //turns off packet handling, just transmits GDO0 pin data
    write_reg_CC1101(CC1101_PKTCTRL0, 0x32);

   
    //setting to 2-FSK, FM mode
    write_reg_CC1101(CC1101_MDMCFG2, 0b01000000);


    //write_reg_CC1101(CC1101_FREQ2, 0x10);
    //write_reg_CC1101(CC1101_FREQ1, 0xB1);
    //write_reg_CC1101(CC1101_FREQ0, 0x3B);
    
    //freq
    //higher byte
    write_reg_CC1101(CC1101_FREQ2, 0x10);
    //middle byte
    write_reg_CC1101(CC1101_FREQ1, 0xA7);
    //lower byte
    write_reg_CC1101(CC1101_FREQ0, 0x62);

    //setting deviation
    //controls volume/bandwidth 0x47 = 47kHz deviation
    //if too low => quiet audio and if too high => distortion.
    write_reg_CC1101(CC1101_DEVIATN, 0x47); 

    //
    write_reg_CC1101(CC1101_MCSM0, 0x18);

    //power
    set_data(CSN_PIN, 0);
    spi_transfer(CC1101_PATABLE); 
    spi_transfer(0xC0);           
    set_data(CSN_PIN, 1);





    //flush the transmit buffer 
    //  i don't know exactly what this does but it is good practice and I think there is like a buffer of data which we want to be empty
    send_strobe(CC1101_SFTX);
    
    //turn on
    send_strobe(CC1101_STX);
}



//maybe remove the magic numbers
//functions from lab3 
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




int main() { //testing wav upload and reading
    // ./dtekv-upload <path to file> 0x20000000
    // 0x20000000 is the start of the general
    // 0x40000000 is the start of the reserved part of the memory, do NOT write to the reserved part

    //nevermind that is wrong, the lecture wasn't using accurate numbers
    //SDRAM (64 MB)	0x00000000-0x03ffffff
    // we will be uploading it to 0x00100000, giving 1 mb to the program before it starts overwriting our saved data
    // ./dtekv-upload <path to file> 0x00100000

    //read the first byte:
    /*int base_file_address = 0x00100000;
    for (int i = 0; i < 4; i++)
    {
        volatile unsigned char* data = (unsigned char*)(base_file_address + i);
        print_bi(*data);
    }

    //get header info
    volatile unsigned int *data = (unsigned int*)base_file_address;
    int data_byte_amount = data[10];
    print_dec(data_byte_amount);*/
    
    int file_amount = 0;
    int current_file = 1;
    int playing = 0;

    


    //setup communication with peripheral
    spi_init();

    //reset peripheral
    reset_CC1101();

    config_CC1101();

    set_direction(GDO0_PIN, 1); 
    
    //transfer_new_file(file_amount);


    volatile unsigned char *file_data = (unsigned char*)FILE_UPLOAD_BASE_ADDRESS;
    //clear the first byte
    //  setting the first byte to 0b11111111 so that we can detect when it changes
    //      i chose 0b11111111 because a wav file should never start with that binary sequence
    file_data[0] = 0b11111111;
    
    
    set_displays(0, 0);
    set_displays(1, -1);
    set_displays(2, -1);
    set_displays(3, -1);
    set_displays(4, 0);
    set_displays(5, -1);

    while (1)
    {
        if (get_btn() && file_amount>0) //button pressed
        {   
            transmit_current_file(current_file);   
        }
        if (is_new_file())
        {
            //there is a new file
            file_amount++;
            transfer_new_file(file_amount);
        }   

        if (file_amount >= get_sw())
        {
            current_file = get_sw();
            //set display for current file
            set_displays(0, current_file);
            //set display for file amount
            set_displays(4, file_amount);

            //clear other displays
            set_displays(1, -1);
            set_displays(2, -1);
            set_displays(3, -1);
            set_displays(5, -1);
            
        }
        delay(10000); //you get weird bugs when you don't have this, probably something stat can't be read or written to that rapidly
    }
}




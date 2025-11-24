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


#define SCL_PIN     (1 << 0)
#define CSN_PIN     (1 << 1)
#define MOSI_PIN    (1 << 4)
#define MISO_PIN    (1 << 6)

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

//  something else that we use for setting the power of the transmission
#define CC1101_PATABLE 0x3E     // PA Power Setting Table

//#define CC1101_IOCFG0 0x02      // GDO0 Output Pin Configuration

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
    set_direction(SCL_PIN | CSN_PIN | MOSI_PIN, 1);
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





int main() {
    //setup communication with peripheral
    spi_init();

    //reset peripheral
    reset_CC1101();
    
    //configure before starting transmission
    //set the 433 frequency
    //  here is how the frequency is calculated
    //      f = ( fxosc / 2^16 ) * control_word
    //  control_word is what we put into the FREQ registers
    //  we want f to be 433 Mhz, fxosc is roughly 26 Mhz (i think, it says so on the chip)
    //      433 = (26 / 2^16) * c
    //      => c = 1 091 426.462
    //      convert to hex: c = 0x10A762
    //higher byte
    write_reg_CC1101(CC1101_FREQ2, 0x10);
    //middle byte
    write_reg_CC1101(CC1101_FREQ2, 0xA7);
    //lower byte
    write_reg_CC1101(CC1101_FREQ2, 0x62);

    //config for OOK (on off keying)
    write_reg_CC1101(CC1101_MDMCFG2, 0x30);
    //write_reg_CC1101(CC1101_IO, 0x2E); //maybe add later

    //set the power output to maximum
    write_reg_CC1101(CC1101_PATABLE, 0xC0);

    //flush the transmit buffer 
    //  i don't know exactly what this does but it is good practice and I think there is like a buffer of data which we want to be empty
    send_strobe(CC1101_SFTX);


    //Play a melody (happy birthday currently):

    //int freqs[] = {261, 261, 294, 261, 349, 330, 261, 261, 294, 261, 392, 349, 261, 261, 523, 440, 349, 330, 294, 440, 440, 349, 392, 349};
    //int freqs[] = {100, 700, 300, 400};
    int freqs[] = {3780, 3780, 3354, 3780, 2827, 2986, 3780, 3780, 3354, 3780, 2515, 2827, 3780, 3780, 1885, 2241, 2827, 2986, 3354, 2241, 2241, 2827, 2515, 2827};
    int timer = 0;
    int on = 1;
    int i = 0;
    


    while (1) {
        for (int j = 0; j < (int)(500000/freqs[i]); j++)
        {
            //turn transmission on
            send_strobe(CC1101_STX);
            
            delay(freqs[i]);

            //turn transmission off
            send_strobe(CC1101_SIDLE);
            
            delay(freqs[i]);
        }
        for (int j = 0; j < (int)(20000/freqs[i]); j++) //silence between notes
        {
            delay(freqs[i]);
            delay(freqs[i]);
        }


        i++;
        if (i >= 24)
        {
            i = 0;
        }
    }



    return 0;
}


extern void print(const char*);
extern void print_dec(const unsigned int);
extern void print_hex32(const unsigned int);

//0x040000e0-0x040000ef (Pins 0-31)
//0x040000f0-0x040000ff (Pins 32-35)

#define GPIO_BASE_ADDRESS 0x040000e0
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
volatile unsigned int* data = (unsigned int*)(GPIO_BASE_ADDRESS + 0 * 0x4);
volatile unsigned int* direction = (unsigned int*)(GPIO_BASE_ADDRESS + 1 * 0x4);


void delay(int amount) { //volatile prevents optimising away loop
    for (volatile int i = 0; i < amount; i++)
    {
        
    }
    
    
}

unsigned char spi_transfer(unsigned char sent){
    unsigned char received = 0x00;
    //we need to send 1 bit 8 times to send 1 byte
    for (int i = 0; i < 8; i++)
    {
        //set MOSI to most significant bit
        if (sent & 0x80) //checks if the most significant bit == 1
        {
            *data |= MOSI_PIN; //set high
        }
        else {
            *data &= ~MOSI_PIN; //set low
        }
        
        sent <<= 1; //shift it to the left now that we have used that bit

        delay(50); //wait for the data line to stabalize

        //scl high
        *data |= SCL_PIN;

        //delay
        delay(1000);

        //read MISO
        received <<= 1;
        if (*data & MISO_PIN) //check if peripheral has sent 1 or 0
        {
            received |= 0x01;
        }
         

        //scl low
        *data &= ~SCL_PIN;
        delay(50); 
    }
    return received;
}


int main() {    
    //GPIO PINS INITIALISATION
    //*direction =    0b00000000000000000000000000010011;
    *direction = SCL_PIN | CSN_PIN | MOSI_PIN; //these are the output pins
    *data = CSN_PIN; //chip select should be high at first

    print_hex32(*data);
    print("\n");
    print_hex32(*direction);
    print("\n");


    //==READING FROM PERIPHERY==:
    //we want to read the VERSION register (0x31)

    //set CSN to LOW
    *data = *data & ~CSN_PIN;

    //send address
    delay(1000);
    spi_transfer(0x30 | 0x80);

    //read response on MISO
    unsigned char received = spi_transfer(0x00);

    //end communicaiton (set CSN to HIGH)
    *data |= CSN_PIN;

    print("\n");
    print_dec(received);

    return 0;
}
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void print_bi(unsigned char byte) {
    for (int i = 0; i < 8; i++)
    {   
        if (byte>>7 == 0b1)
        {
            printf("1");
        }
        else{
            printf("0");
        }
        byte = byte << 1;
    }
    printf("\n");
}

int main(){
    FILE *file = fopen("./audio/voice_test_compressed_8_bit_unsigned_22k_V2.wav", "rb");
    
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    unsigned char header[44];

        size_t bytesRead = fread(header, sizeof(char), 44, file);



    for (int i = 0; i < bytesRead; i++)
    {
        printf("i=%d\n", i);
        print_bi(header[i]);
        //printf("%d\n",(int)header[i]-128);
        if (i==43)
        {
            printf("\n");
        }
        
    }
    printf("bytes read: %d", bytesRead);
    
    return 0;
}
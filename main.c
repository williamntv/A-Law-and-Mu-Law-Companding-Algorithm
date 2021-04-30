#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include "stdint.h"
#include <inttypes.h>

#define ALAW_MAX 0xFFF

#define PRINT_DEBUG_STATEMENTS

#ifdef PRINT_DEBUG_STATEMENTS
#define debug_printf printf
#else
#define debug_printf(...)
#endif

/*
* Description:
* Encodes a 12-bit signed integer using the A-Law.
* Parameters:
* sample - the number that will be encoded
* Returns:
* The encoded number
*/
inline uint8_t ALaw_Encode(register int16_t sample) {
    register uint16_t tableRow;
    register uint16_t zero = 0;
    register uint8_t sign = 0;
    register uint8_t abcdShift = 0;
       
    __asm__ __volatile__("cmp\t %1, $0\n"
                          "movlt\t %0, $0x80\n"
                          "movge\t %0, $0x00\n"
                          : "=r" (sign)
                          : "r" (sample));
   
    __asm__ __volatile__("cmp\t %2, %1\n"
                          "sublt\t %0, %1, %2\n"
                          "addge\t %0, %1, %2\n"
                          : "=r" (sample)
                          : "r" (zero), "r" (sample));                   

    if (sample > ALAW_MAX) {
        //debug_printf("Input value is larger than 12 bits!!!\n"); // Cap input to avoid overflow
        sample = ALAW_MAX;
    }
    
    if (sample & 0x800) {
        tableRow = 7;
    }
    else if (sample & 0x400) {
        tableRow = 6;
    }
    else if (sample & 0x200) {
        tableRow = 5;
    }
    else if (sample & 0x100) {
        tableRow = 4;
    }
    else if (sample & 0x080) {
        tableRow = 3;
    }
    else if (sample & 0x040) {
        tableRow = 2;
    }
    else if (sample & 0x020) {
        tableRow = 1;
    }
    else {
        tableRow = 0;
    }
    
    abcdShift = tableRow;

    if (tableRow == 0) {
        abcdShift = 1;
    }

    register uint8_t abcd = (sample >> abcdShift) & 0xF;
    register uint8_t chord = tableRow << 4;
    register uint8_t transmittedValue = (sign | chord | abcd) ^ 0x55;

    return (transmittedValue);
    
}

/*
* Description:
* Decodes an 8-bit signed integer using the A-Law.
* Parameters:
* sample - the number that will be decoded
* Returns:
* The decoded number
*/
inline int16_t ALaw_Decode(register uint8_t sample) {
    register int8_t sign = 0x00;
    register int16_t decoded = 0;

    sample ^= 0x55;

    if ((sample & 0x80) == 0x80) {
        sample &= 0x7F;
        sign = -1;
    }
    else {
        sign = 0;
    }

    register uint8_t tableRow = ((sample & 0x70) >> 4);
    register uint8_t abcdShift = tableRow;
    register int16_t oneBit = 1 << (4 + tableRow);

    switch (tableRow) {
    case 0:
        abcdShift = 0;
        oneBit = 0x00;
        break;
    case 1:
        abcdShift = 0;
        break;
    default:
        abcdShift = tableRow - 1;
        break;
    }

    register int16_t abcd = (((sample & 0x0F) << 1) | 1) << abcdShift;

    decoded = oneBit | abcd;

    if (sign == -1) {
        decoded = -decoded;
    }

    return (decoded);
}

//=================================Test==========================================//
inline void test(register int16_t sample) {
    register uint8_t encoded_sample;
    register int16_t decoded_sample;

    debug_printf("==== SAMPLE: 0x%04hx \n", sample);
    encoded_sample = ALaw_Encode(sample);
    debug_printf("------------\n");
    debug_printf("Encoded Sample: 0x%02x \n", encoded_sample);

    decoded_sample = ALaw_Decode(encoded_sample);
    debug_printf("------------\n");
    debug_printf("Decoded Sample: 0x%04hx \n", decoded_sample);

    debug_printf("------------\n");

    if (sample == decoded_sample) {
        debug_printf("Input sample matches output sample:   0x%x == 0x%x\n", sample, decoded_sample);
        debug_printf("The decimal values:  %d == %d\n", sample, decoded_sample);
    }
    else {
        debug_printf("Not match!   0x%x =/= 0x%x\n", sample, decoded_sample);
        debug_printf("The decimal values:  %d =/= %d\n", sample, decoded_sample);
    }

    debug_printf("==~~~~~~~~~~~~~~~~~~~==\n\n");
}

//=================================Main.c========================================//
int main(void) {
    int16_t sample;
    int i = 1;

    //Tests:
    debug_printf("==============\n");
    debug_printf("Tests:\n");
    debug_printf("==============\n\n");

	while (i > 0 ) {
    	sample = -2460;
    	test(sample);
    
    	sample = -1505;
    	test(sample);
    
    	sample = -650;
    	test(sample);
    
    	sample = -338;
    	test(sample);
    
    	sample = -90;
    	test(sample);
    
    	sample = -1;
    	test(sample);
    
    	sample = 40;
    	test(sample);
    
    	sample = 102;
    	test(sample);
    
    	sample = 169;
    	test(sample);
    
    	sample = 420;
    	test(sample);
    
    	sample = 499;
    	test(sample);
    
    	sample = 980;
    	test(sample);
    	i--;
    }

    return 0;
}
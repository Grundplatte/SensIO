//
// Created by pi on 11/21/17.
//

#include <cmath>
#include <cstdio>
#include "Berger.h"

/*
 * Generate Berger code (4bit)
 */
// TODO: support longer codes
int Berger::generate(byte *input, int length, byte *output) {

    unsigned int i, count, checkBits;

    checkBits = ceil(log2(length + 1));
    if(checkBits > MAX_CHECK_LENGTH) {
        printf("[E] Berger codes with more than %i bit are not supported!\n", MAX_CHECK_LENGTH);
        return -1;
    }

    count = 0;
    for(i=0; i<length; i++){
        if(input[length/8] & (1<<(i%8)))
            count++;
    }

    *output = (byte)count&0x0F;

    return checkBits;
}

int Berger::check(byte *input, int length) {
    // TODO: implement
    return 0;
}

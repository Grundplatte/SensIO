//
// Created by pi on 11/21/17.
//

#include "Hadamard.h"

Hadamard::Hadamard() {
    std::shared_ptr<spdlog::logger> log;
    log = spd::get("Had");

    if (log)
        m_log = log;
    else
        m_log = spd::stdout_color_mt("Had");
}

Hadamard::~Hadamard() {}

/*
 * Hadamard Code
 * generate 8bit code from 3bit sqn
 */

//uint8_t G = [0b00001111, 0b00110011, 0b01010101]; // Generator matrix for k=3
// TODO: support variable input length (only 3->8 is supported for now)
int Hadamard::encode(byte *input, int length, byte *output)
{
    if(length != 3)
        return(-1);

    // inflate => repeat every bit 8 times
    int i;
    word x = 0;
    for(i=0; i<3; i++) {
        if(*input & (1<<i))
            x |= 0xFF;
        x <<= 8;
    }
    x >>= 8; // bad practice, but x is is big enough

    // multiplication
    unsigned int P = G & x;

    // addition
    *output = 0;
    P ^= P>>16;
    P ^= P>>8;
    *output = P & 0xFF;

    return 8;
}

// simple version of decoder
// TODO: support variable input length (only 8->3 is supported for now)
int Hadamard::decode(byte *input, int length, byte *output)
{
    if(length != 8)
        return(-1);

    unsigned int i;
    int lowHam, ham;
    lowHam = 1000;
    for(i=0; i<8; i++) {
        ham = calcHamming(*input, H[i]);
        if(ham < lowHam) {
            lowHam = ham;
            *output = i;
        }
    }

    return 3;
}

// TODO: move to helper class
// TODO: support bigger data types / input data
int Hadamard::calcHamming(byte input1, byte input2)
{
    unsigned int i;
    int ham;
    ham = 0;
    for(i=0; i<8; i++) {
        if((input1 & (1<<i)) != (input2 & (1<<i))) {
            ham++;
        }
    }
    return ham;
}

// TODO: make dynamic
int Hadamard::getEncodedSize() {
    return 7;

}

//
// Created by pi on 11/21/17.
//

#pragma once

#include "ECC.h"

class Hadamard : public ECC {
public:
    int encode(byte *input, int length, byte *output) override;
    int decode(byte *input, int length, byte *output) override;

private:
    word G = 0b000011110011001101010101; // Generator matrix (as a vector)
    byte H[8] = {0b00000000, 0b00001111, 0b00110011, 0b00111100,
                 0b01010101, 0b01011010, 0b01100110, 0b01101001};
    int calcHamming(byte in1, byte in2);
};

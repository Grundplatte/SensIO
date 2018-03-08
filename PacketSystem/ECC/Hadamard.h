//
// Created by pi on 11/21/17.
//

#pragma once

#include "ECC.h"

class Hadamard : public ECC {
public:
    Hadamard();

    ~Hadamard();

    int encode(byte_t *input, size_t length, byte_t *output) override;

    int decode(byte_t *input, size_t length, byte_t *output) override;

    int check(byte_t *input, size_t length) override;

    int getEncodedSize(size_t length) override;

private:
    // Hardcoded Hadamard codes
    byte_t _H2[2] = {0b00, 0b01};
    byte_t _H4[4] = {0b0000, 0b0011, 0b0101, 0b0110};
    byte_t _H8[8] = {0b00000000, 0b00001111, 0b00110011, 0b00111100,
                 0b01010101, 0b01011010, 0b01100110, 0b01101001};

    int calcHamming(byte_t in1, byte_t in2);
};

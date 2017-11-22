//
// Created by pi on 11/21/17.
//

#pragma once

#include "../../Defines.h"

class ECC {
public:
    /**
     * Generate a error correction code from the input
     * @param input: payload
     * @param length: input length in byte (TODO: change to bit)
     * @param output: encoded payload
     * @return: output length in byte (-1 on error)(TODO: change to bit)
     **/
    virtual int encode(byte *input, int length, byte *output) = 0;

    /**
     * Try to decode the encoded payload
     * @param input: encoded payload
     * @param length: input length in byte (TODO: change to bit)
     * @param output: decoded payload
     * @return: output length in byte (-1 on error)(TODO: change to bit)
     **/
    virtual int decode(byte *input, int length, byte *output) = 0;
};

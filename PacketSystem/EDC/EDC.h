//
// Created by pi on 11/21/17.
//

#pragma once

#include "../../Defines.h"

class EDC {
public:
    /**
     * Generate a error detection code from the input
     * @param input: payload
     * @param length: input length in byte (TODO: change to bit)
     * @param output: edc
     * @return: output length in byte (TODO: change to bit)
     **/
    virtual int generate(byte *input, int length, byte *output) = 0;

    /**
     * Validate the payload + error detection code
     * @param input: payload + edc
     * @param length: input length in byte (TODO: change to bit)
     * @return: valid (0) or invalid(-1)
     **/
    virtual int check(byte *input, int length) = 0;
};
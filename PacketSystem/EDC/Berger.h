//
// Created by pi on 11/21/17.
//

#pragma once

#include "EDC.h"

class Berger: public EDC {
public:
    /*
     * Generate a berger code from the input
     * input: payload
     * output: payload + berger code
     * length: input length in byte (TODO: change to bit)
     * return: output length in byte (TODO: change to bit)
     */
    int generate(byte *input, int length, byte *output) override;

    /*
     * Validate the payload + berger code
     * input: payload + berger code
     * length: input length in byte (TODO: change to bit)
     * return: valid (0) or invalid(-1)
     */
    int check(byte *input, int length) override;
private:
    static const int MAX_CHECK_LENGTH = 4;
};
//
// Created by pi on 11/21/17.
//

#pragma once

#include <bitset>
#include "EDC.h"

class Berger: public EDC {
public:
    Berger();

    ~Berger();

    /*
     * Generate a berger code from the input
     * input: payload
     * output: payload + berger code
     * length: input length in byte (TODO: change to bit)
     * return: output length in byte (TODO: change to bit)
     */
    int generate(std::vector<bit> input, std::vector<bit> &output) override;

    /*
     * Validate the payload + berger code
     * input: payload + berger code
     * length: input length in byte (TODO: change to bit)
     * return: valid (0) or invalid(-1)
     */
    int check(std::vector<bit> input, int datasize) override;

    int calcOutputSize(unsigned int length) override;

private:
    static const int MAX_CHECK_LENGTH;
};
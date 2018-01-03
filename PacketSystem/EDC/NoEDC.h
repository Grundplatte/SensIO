//
// Created by pi on 11/22/17.
//

#pragma once

#include "EDC.h"

class NoEDC : public EDC {
public:
    NoEDC();

    ~NoEDC();

    int generate(std::vector<bit_t> input, std::vector<bit_t> &output) override;

    int check(std::vector<bit_t> input, std::vector<bit_t> edc_in) override;

    int calcOutputSize(unsigned int length) override;
};

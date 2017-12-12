//
// Created by pi on 11/22/17.
//

#pragma once

#include "EDC.h"

class NoEDC : public EDC {
public:
    NoEDC();

    ~NoEDC();

    int generate(std::vector<bit> input, std::vector<bit> &output) override;

    int check(std::vector<bit> input, int length) override;

    int calcOutputSize(unsigned int length) override;
};

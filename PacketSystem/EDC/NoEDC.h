//
// Created by pi on 11/22/17.
//

#pragma once

#include "EDC.h"

class NoEDC : public EDC {
public:
    NoEDC();

    ~NoEDC();

    int generate(byte *input, unsigned int length, byte *output) override;

    int check(byte *input, unsigned int length) override;

    int calcOutputSize(unsigned int length) override;
};

//
// Created by pi on 11/22/17.
//

#pragma once

#include "ECC.h"
#include "../../Defines.h"

class NoECC : public ECC {
    NoECC();

    ~NoECC();

    int encode(byte *input, size_t length, byte *output) override;

    int decode(byte *input, size_t length, byte *output) override;

    // only error detection
    int check(byte *input, size_t length) override;

public:
    int getEncodedSize(size_t length) override;
};

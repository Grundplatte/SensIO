//
// Created by pi on 11/22/17.
//

#pragma once

#include "ECC.h"
#include "../../Defines.h"

class NoECC : public ECC {
public:

    NoECC();

    ~NoECC();

    int encode(byte_t *input, size_t length, byte_t *output) override;

    int decode(byte_t *input, size_t length, byte_t *output) override;

    // only error detection
    int check(byte_t *input, size_t length) override;
    int getEncodedSize(size_t length) override;
};

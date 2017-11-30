//
// Created by pi on 11/22/17.
//

#pragma once

#include "ECC.h"
#include "../../Defines.h"

class NoECC : public ECC {
    NoECC();

    ~NoECC();
    int encode(byte *input, int length, byte *output) override;
    int decode(byte *input, int length, byte *output) override;

public:
    int getEncodedSize() override;
};

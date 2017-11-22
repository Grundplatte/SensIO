//
// Created by pi on 11/22/17.
//

#pragma once

#include "EDC.h"

class NoEDC : public EDC {
    int generate(byte *input, int length, byte *output) override;

    int check(byte *input, int length) override;
};

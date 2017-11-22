//
// Created by pi on 11/22/17.
//

#include <cstring>
#include "NoEDC.h"

int NoEDC::generate(byte *input, int length, byte *output) {
    memcpy(output, input, length);
    return length;
}

int NoEDC::check(byte *input, int length) {
    return 0;
}

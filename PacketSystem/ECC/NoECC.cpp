//
// Created by pi on 11/22/17.
//

#include <cstring>
#include "NoECC.h"

int NoECC::encode(byte *input, int length, byte *output) {
    memcpy(output, input, length);
    return length;
}

int NoECC::decode(byte *input, int length, byte *output) {
    memcpy(output, input, length);
    return length;
}

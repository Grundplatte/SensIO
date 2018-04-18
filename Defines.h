/**
    SensIO
    Defines.h

    Global used definitions.

    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_DEFINES_H
#define SIDECHANNEL_DEFINES_H

#include <cmath>

#define CYCLE_MS 80 // ms (1 cycle)

#define STATUS_OK 0
#define DYNAMIC

// Error codes
#define TIMEOUT_WHILE_RECEIVING -1
#define TIMEOUT_WHILE_WAITING   -2

// c++ cant handle single bits too well (referencing, etc.)
typedef unsigned char bit_t;

typedef unsigned char byte_t;
typedef unsigned int word_t;

// optimal for 2 sqn bits with berger edc
//const unsigned int P_DATA_BITS[6] = {1, 5, 13, 29, 61, 125}; // (2/3/4/5/6 checkbits)
const unsigned int P_DATA_BITS[3] = {5, 29, 61};
const unsigned int P_SQN_BITS = 2;
const unsigned int P_CMD_BITS = 2;

const unsigned int P_TEST_UPSCALE = 2;
const unsigned int P_INIT_SCALE = 0;

const int MAX_SQN = (int) pow(2, P_SQN_BITS) - 1;

#endif // SIDECHANNEL_DEFINES_H
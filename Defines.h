#pragma once

#include <cmath>

#define CYCLE_MS 80 // ms (1 cycle)

// c++ cant handle single bits too well (referencing, etc.)
typedef unsigned char bit_t;

typedef unsigned char byte;
typedef unsigned int word;

// optimal for 2 sqn bits with berger edc
const unsigned int P_DATA_BITS[5] = {1, 5, 13, 29, 61}; // (2/3/4/5/6 checkbits)
const unsigned int P_SQN_BITS = 2;
const unsigned int P_CMD_BITS = 2;

const unsigned int P_TEST_UPSCALE = 2;

const unsigned int CYCLE_DELAY = (CYCLE_MS % 1000) * 1000000;

const int MAX_SQN = (int) pow(2, P_SQN_BITS) - 1;
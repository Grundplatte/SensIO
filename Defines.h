#pragma once

#include <cmath>

#define MAXDELAY_MS 80 // ms (1cycle)

// c++ cant handle single bits too well (referencing, etc.)
typedef unsigned char bit_t;

typedef unsigned char byte;
typedef unsigned int word;

const unsigned int P_DATA_BITS = 12;
const unsigned int P_SQN_BITS = 3;
const unsigned int P_CMD_BITS = 2;

const unsigned int P_MAXDELAY = (MAXDELAY_MS % 1000) * 1000000;

const int MAX_SQN = (int) pow(2, P_SQN_BITS) - 1;
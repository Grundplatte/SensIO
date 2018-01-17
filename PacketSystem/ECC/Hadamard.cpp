//
// Created by pi on 11/21/17.
//

#include "Hadamard.h"

Hadamard::Hadamard() {
    std::shared_ptr<spdlog::logger> log;
    log = spd::get("Had");

    if (log)
        m_log = log;
    else
        m_log = spd::stdout_color_mt("Had");
}

Hadamard::~Hadamard() {}

/*
 * Hadamard Code
 * generate 2/4/8bit code from 1/2/3bit sqn
 */

int Hadamard::encode(byte *input, size_t length, byte *output) {
    int result;

    switch (length) {
        case 1:
            *output = H2[*input & 0x03];
            result = 2;
            break;

        case 2:
            *output = H4[*input & 0x07];
            result = 4;
            break;

        case 3:
            *output = H8[*input & 0x0F];
            result = 8;
            break;

        default:
            result = -1;
            m_log->debug("Encode: Wrong input length: {}", length);
    }

    return result;
}

// simple version of decoder
int Hadamard::decode(byte *input, size_t length, byte *output)
{
    unsigned int i;
    unsigned char *H;
    int lowHam = 1000;
    int ham = 0;

    switch (length) {
        case 2:
            H = H2;
            break;

        case 4:
            H = H4;
            break;

        case 8:
            H = H8;
            break;

        default:
            m_log->debug("Decode: Wrong input length: {}", length);
            return -1;
    }

    for (i = 0; i < length; i++) {
        ham = calcHamming(*input, H[i]);

        if(ham < lowHam) {
            lowHam = ham;
            *output = i;
        }
    }

    m_log->trace("Hadamard decoded: {0x:} => {1:x}", input, output);

    return 3;
}

// TODO: support larger inputs
int Hadamard::calcHamming(byte input1, byte input2)
{
    unsigned int i;
    int ham;
    ham = 0;
    for(i=0; i<8; i++) {
        if((input1 & (1<<i)) != (input2 & (1<<i))) {
            ham++;
        }
    }
    return ham;
}

int Hadamard::getEncodedSize(size_t length) {
    switch (length) {
        case 1:
            return 2;

        case 2:
            return 4;

        case 3:
            return 8;

        default:
            m_log->debug("GetSize: Wrong input length: {}", length);
            return -1;
    }
}

int Hadamard::check(byte *input, size_t length) {

    unsigned char *H;
    switch (length) {
        case 2:
            H = H2;
            break;

        case 4:
            H = H4;
            break;

        case 8:
            H = H8;
            break;

        default:
            m_log->debug("Decode: Wrong input length: {}", length);
            return -1;
    }

    for (int i = 0; i < length; i++) {
        if (memcmp(input, H + i, 1) == 0) {
            m_log->debug("Found matching SQN: {}", i);
            return i;
        }
    }

    return -1;
}

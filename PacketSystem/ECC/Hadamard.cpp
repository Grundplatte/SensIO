//
// Created by pi on 11/21/17.
//

#include "Hadamard.h"

Hadamard::Hadamard() {
    std::shared_ptr<spdlog::logger> log = spd::get("Had");
    _log = log ? log : spd::stdout_color_mt("Had");
}

Hadamard::~Hadamard() = default;

/*
 * Hadamard Code
 * generate 2/4/8bit code from 1/2/3bit sqn
 */

int Hadamard::encode(byte_t *input, size_t length, byte_t *output) {
    int result;

    switch (length) {
        case 1:
            *output = _H2[*input & 0x03];
            result = 2;
            break;

        case 2:
            *output = _H4[*input & 0x07];
            result = 4;
            break;

        case 3:
            *output = _H8[*input & 0x0F];
            result = 8;
            break;

        default:
            result = -1;
            _log->debug("Encode: Wrong input length: {}", length);
    }

    return result;
}

// simple version of decoder
int Hadamard::decode(byte_t *input, size_t length, byte_t *output)
{
    unsigned char *H;

    switch (length) {
        case 2:
            H = _H2;
            break;

        case 4:
            H = _H4;
            break;

        case 8:
            H = _H8;
            break;

        default:
            _log->debug("Decode: Wrong input length: {}", length);
            return -1;
    }

    int ham = 0;
    int lowHam = 1000;
    for (int i = 0; i < length; i++) {
        ham = calcHamming(*input, H[i]);

        if(ham < lowHam) {
            lowHam = ham;
            *output = (unsigned char) i;
        }
    }

    _log->trace("Hadamard decoded: {0x:} => {1:x}", input, output);

    return 3;
}

// TODO: support larger inputs
int Hadamard::calcHamming(byte_t input1, byte_t input2)
{
    int ham = 0;
    for (int i = 0; i < 8; i++) {
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
            _log->debug("GetSize: Wrong input length: {}", length);
            return -1;
    }
}

int Hadamard::check(byte_t *input, size_t length) {

    unsigned char *H;
    switch (length) {
        case 2:
            H = _H2;
            break;

        case 4:
            H = _H4;
            break;

        case 8:
            H = _H8;
            break;

        default:
            _log->debug("Decode: Wrong input length: {}", length);
            return -1;
    }

    for (int i = 0; i < length; i++) {
        if (memcmp(input, H + i, 1) == 0) {
            _log->debug("Found matching SQN: {}", i);
            return i;
        }
    }

    return -1;
}

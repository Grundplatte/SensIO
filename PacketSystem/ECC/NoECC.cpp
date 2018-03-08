//
// Created by pi on 11/22/17.
//

#include <cstring>
#include "NoECC.h"

NoECC::NoECC() {
    std::shared_ptr<spdlog::logger> log = spd::get("NoECC");
    _log = log ? log : spd::stdout_color_mt("NoECC");
}

NoECC::~NoECC() = default;

int NoECC::encode(byte_t *input, size_t length, byte_t *output) {
    memcpy(output, input, length);
    return length;
}

int NoECC::decode(byte_t *input, size_t length, byte_t *output) {
    memcpy(output, input, length);
    return length;
}

int NoECC::getEncodedSize(size_t length) {
    return 0;
}

int NoECC::check(byte_t *input, size_t length) {
    return 0;
}

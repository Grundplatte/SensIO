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

int NoECC::encode(byte *input, size_t length, byte *output) {
    memcpy(output, input, length);
    return length;
}

int NoECC::decode(byte *input, size_t length, byte *output) {
    memcpy(output, input, length);
    return length;
}

// FIXME: return correct size!
int NoECC::getEncodedSize(size_t length) {
    return 0;
}

int NoECC::check(byte *input, size_t length) {
    return 0;
}

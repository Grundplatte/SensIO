//
// Created by pi on 11/22/17.
//

#include <cstring>
#include "NoECC.h"

NoECC::NoECC() {
    std::shared_ptr<spdlog::logger> log;
    log = spd::get("NoECC");

    if (log)
        m_log = log;
    else
        m_log = spd::stdout_color_mt("NoECC");
}

NoECC::~NoECC() {}

int NoECC::encode(byte *input, int length, byte *output) {
    memcpy(output, input, length);
    return length;
}

int NoECC::decode(byte *input, int length, byte *output) {
    memcpy(output, input, length);
    return length;
}

// FIXME: return correct size!
int NoECC::getEncodedSize() {
    return 0;
}

int NoECC::check(byte *input, int length) {
    return 0;
}

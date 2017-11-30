//
// Created by pi on 11/21/17.
//

#include <cmath>
#include <cstdio>
#include "Berger.h"

const int Berger::MAX_CHECK_LENGTH = 4;

Berger::Berger() {
    std::shared_ptr<spdlog::logger> log;
    log = spd::get("Berger");

    if (log)
        m_log = log;
    else
        m_log = spd::stdout_color_mt("Berger");
}

Berger::~Berger() {}

// TODO: support longer codes
int Berger::generate(byte *input, unsigned int length, byte *output) {

    unsigned int i, count, checkBits;

    checkBits = calcOutputSize(length);
    if(checkBits > MAX_CHECK_LENGTH) {
        m_log->error("Berger codes with more than {0} bit are not supported!", MAX_CHECK_LENGTH);
        return -1;
    }

    count = 0;
    for(i=0; i<length; i++){
        if(input[length/8] & (1<<(i%8)))
            count++;
    }

    *output = (byte)count&0x0F;

    return checkBits;
}

int Berger::check(byte *input, unsigned int length) {
    // TODO: implement
    return 0;
}

int Berger::calcOutputSize(unsigned int length) {
    return ceil(log2(length + 1));
}

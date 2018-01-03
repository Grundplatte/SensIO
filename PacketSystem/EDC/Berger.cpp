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
int Berger::generate(std::vector<bit_t> input, std::vector<bit_t> &output) {

    int checkBits = calcOutputSize(input.size());

    m_log->trace("Generating {0} checkbits.", checkBits);

    unsigned int count = 0;
    for (int i = 0; i < input.size(); i++) {
        if (input[i])
            count++;
    }

    for (int i = 0; i < checkBits; i++) {
        output.push_back((bit_t) ((count & (1 << i)) >> i));
    }

    return checkBits;
}

int Berger::check(std::vector<bit_t> input, std::vector<bit_t> edc_in) {
    std::vector<bit_t> berger_calc;

    // generate reference berger code and confirm the bitsize of the error code
    generate(input, berger_calc);
    if (berger_calc.size() != edc_in.size()) {
        m_log->warn("Checkbits: size not matching.");
        return -3;
    }

    for (int i = 0; i < edc_in.size(); i++) {
        if (edc_in[i] != berger_calc[i]) {
            m_log->warn("Bits don't match: input({0:x}), calc({1:x}), ind({2})", edc_in[i], berger_calc[i],
                        i);
            return -1;
        }
    }

    return 1;
}

int Berger::calcOutputSize(unsigned int length) {
    return (int) ceil(log2(length + 1));
}

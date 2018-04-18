/**
    SensIO
    Berger.cpp

    Simple implementation of the Berger code.

    @author Markus Feldbacher
*/

#include <cmath>
#include <cstdio>
#include "Berger.h"

Berger::Berger() {
    std::shared_ptr<spdlog::logger> log = spd::get("Berger");
    _log = log ? log : spd::stdout_color_mt("Berger");
}

Berger::~Berger() = default;

int Berger::generate(std::vector<bit_t> input, std::vector<bit_t> &output) {

    int check_bits = calcOutputSize(input.size());

    _log->trace("Generating {0} checkbits.", check_bits);

    unsigned int count = 0;
    for (unsigned char i : input) {
        if (i)
            count++;
    }

    for (int i = 0; i < check_bits; i++) {
        output.push_back((bit_t) ((count & (1 << i)) >> i));
    }

    return check_bits;
}

int Berger::check(std::vector<bit_t> input, std::vector<bit_t> edc_in) {
    std::vector<bit_t> berger_calc;

    // generate reference berger code and confirm the bitsize of the error code
    generate(input, berger_calc);
    if (berger_calc.size() != edc_in.size()) {
        _log->warn("Checkbits: size not matching.");
        return -3;
    }

    for (int i = 0; i < edc_in.size(); i++) {
        if (edc_in[i] != berger_calc[i]) {
            _log->warn("Bits don't match: input({0:x}), calc({1:x}), ind({2})", edc_in[i], berger_calc[i],
                       i);
            return -1;
        }
    }

    return 1;
}

int Berger::calcOutputSize(unsigned int length) {
    return (int) ceil(log2(length + 1));
}

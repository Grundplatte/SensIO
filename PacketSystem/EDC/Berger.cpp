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
int Berger::generate(std::vector<bit> input, std::vector<bit> &output) {

    int checkBits = calcOutputSize(input.size());
    if(checkBits > MAX_CHECK_LENGTH) {
        m_log->error("Berger codes with more than {0} bit are not supported!", MAX_CHECK_LENGTH);
        return -1;
    }

    unsigned int count = 0;
    for (unsigned int i = 0; i < input.size(); i++) {
        if (input.at(i))
            count++;
    }

    output = input;
    for (unsigned int i = 0; i < checkBits; i++) {
        output.push_back((bit) (count & (1 << i)));
    }

    return checkBits;
}

int Berger::check(std::vector<bit> input, int datasize) {
    // TODO: implement
    std::vector<bit> input_data(input.begin(), input.begin() + datasize);
    std::vector<bit> berger_calc;

    // generate reference berger code
    int checkBits = generate(input_data, berger_calc);

    if (checkBits != input.size() - datasize) {

        m_log->warn("Checkbits({0}) != {1}", checkBits, input.size() - datasize);
        return -3;
    }

    int berger_index = 0;
    for (int i = datasize; i < input.size(); i++) {
        if (input[i] != berger_calc[berger_index++]) {
            m_log->warn("Check: input({0:x}), calc({1:x}), ind({2})", input[i], berger_calc[berger_index - 1],
                        berger_index - 1);
            return -1;
        }
    }

    return 1;
}

int Berger::calcOutputSize(unsigned int length) {
    return (int) ceil(log2(length + 1));
}

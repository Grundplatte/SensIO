//
// Created by pi on 11/22/17.
//

#include <cstring>
#include "NoEDC.h"

NoEDC::NoEDC() {
    std::shared_ptr<spdlog::logger> log;
    log = spd::get("NoEDC");

    if (log)
        m_log = log;
    else
        m_log = spd::stdout_color_mt("NoEDC");
}

NoEDC::~NoEDC() {}


int NoEDC::generate(std::vector<bit> input, std::vector<bit> &output) {
    output = input;
    return output.size();
}

int NoEDC::check(std::vector<bit> input, int length) {
    return 0;
}

int NoEDC::calcOutputSize(unsigned int length) {
    return length;
}
//
// Created by pi on 11/22/17.
//

#include <cstring>
#include "NoEDC.h"

NoEDC::NoEDC() {
    std::shared_ptr<spdlog::logger> log = spd::get("NoEDC");
    _log = log ? log : spd::stdout_color_mt("NoEDC");
}

NoEDC::~NoEDC() = default;


int NoEDC::generate(std::vector<bit_t> input, std::vector<bit_t> &output) {
    output = input;
    return output.size();
}

int NoEDC::check(std::vector<bit_t> input, std::vector<bit_t> edc_in) {
    return 0;
}

int NoEDC::calcOutputSize(unsigned int length) {
    return length;
}
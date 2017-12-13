//
// Created by pi on 11/21/17.
//

#pragma once

#include "../../spdlog/spdlog.h"
#include "../../Defines.h"

namespace spd = spdlog;

class EDC {
public:
    /**
     * Generate a error detection code from the input
     * @param input: payload
     * @param length: input length in bits
     * @param output: edc
     * @return: output length in bits
     **/
    virtual int generate(std::vector<bit_t> input, std::vector<bit_t> &output) = 0;

    /**
     * Validate the payload + error detection code
     * @param input: payload + edc
     * @param length: input length in bits
     * @return: valid (0) or invalid(-1)
     **/
    virtual int check(std::vector<bit_t> input, int datasize) = 0;

    /**
     * Calculate the size of the output code in bits
     * @param length: input length in bits
     * @return: output length in bits
     **/
    virtual int calcOutputSize(unsigned int length) = 0;

protected:
    std::shared_ptr<spd::logger> m_log;
};
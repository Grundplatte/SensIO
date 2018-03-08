//
// Created by pi on 11/21/17.
//

#pragma once

#include "../../spdlog/spdlog.h"
#include "../../Defines.h"

namespace spd = spdlog;

class ECC {
public:
    /**
     * Generate a error correction code from the input
     * @param input: payload
     * @param length: input length in bit
     * @param output: encoded payload
     * @return: output length in bit (-1 on error)
     **/
    virtual int encode(byte_t *input, size_t length, byte_t *output) = 0;

    /**
     * Try to decode the encoded payload
     * @param input: encoded payload
     * @param length: input length in bit
     * @param output: decoded payload
     * @return: output length in bit (-1 on error)
     **/
    virtual int decode(byte_t *input, size_t length, byte_t *output) = 0;

    virtual int check(byte_t *input, size_t length) = 0;

    virtual int getEncodedSize(size_t) = 0;

protected:
    std::shared_ptr<spd::logger> _log;
};

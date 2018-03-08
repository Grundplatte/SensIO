#pragma once

#include "../spdlog/spdlog.h"
#include "../Defines.h"
#include "HAL/HAL.h"

namespace spd = spdlog;

class Sensor {
public:
    /**
     * Check if the sensor is activated
     * @return: -1 on error, 0 otherwise
     **/
    virtual int isActive() = 0;

    /**
     * Toggle the sensor on/off
     * @return: -1 on error, 0 otherwise
     **/
    virtual int toggleOnOff(bit_t onOff) = 0;

    /**
     * Wait until the sensor is ready to transmit data
     * @return: -1 on error, 0 otherwise
     **/
    virtual int waitForSensReady() = 0;

    /**
     * Send one bit over the covert channel
     * @param bit: Bit to send
     * @return: -1 on error, 0 otherwise
     **/
    virtual int sendBit(bit_t bit) = 0;

    virtual int sendByte(unsigned char byte) = 0;

    /**
     * Read a bit from the covert channel
     * @param timeout: use timeout (only used during a transmission, not for the first bit)
     * @param long_timeout: 0...disabled, otherwise X * normal timeout (usually 2 or 3)
     * @return: -1 on error, 0 or 1 otherwise (depending on transmitted bit)
     **/
    virtual int readBit(bool timeout, int long_timeout) = 0;

    virtual int readByte() = 0;

    /**
     * Send a reset (read both result registers)
     * @return: (-1 on error, 0 otherwise)
     **/
    virtual int sendReset() = 0;

    virtual int supportsBytes() = 0;

protected:
    std::shared_ptr<spd::logger> _log;
    std::shared_ptr<HAL> _hal;
};
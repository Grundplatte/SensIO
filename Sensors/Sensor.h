#pragma once

#include "../spdlog/spdlog.h"
#include "../Defines.h"

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
     * Send one bit over the covert channel TODO: move to own class?
     * @param bit: Bit to send
     * @return: -1 on error, 0 otherwise
     **/
    virtual int sendBit(bit_t bit) = 0;

    /**
     * Try to read a bit from the covert channel TODO: move to own class?
     * @return: -1 on error, 0 or 1 otherwise (depending on transmitted bit)
     **/
    virtual int tryReadBit() = 0;

    /**
     * Send a reset (read both result registers) TODO: move to own class?
     * @return: (-1 on error, 0 otherwise)
     **/
    virtual int sendReset() = 0;

protected:
    std::shared_ptr<spd::logger> m_log;
};
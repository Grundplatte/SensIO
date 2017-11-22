#pragma once

#include "../Defines.h"

class Sensor {
public:
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
    virtual int sendBit(byte bit) = 0;

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
};
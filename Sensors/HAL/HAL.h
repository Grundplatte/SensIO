#pragma once

#include "../../Defines.h"

class HAL {
public:
    /**
     * Generic read from sensor
     * @param slaveAddr: Address of the sensor slave
     * @param regAddr: Address of the target register
     * @param length: Number of bytes to read
     * @param data: Data buffer to store the resulting data
     * @return: (-1 on error, 0 otherwise)
     **/
    virtual int read(byte slave_addr, byte regAddr, unsigned int length, byte *data) = 0;

    /**
     * Generic write to sensor
     * @param slaveAddr: Address of the sensor slave
     * @param regAddr: Address of the target register
     * @param length: Number of bytes to write
     * @param data: Data to write
     * @return: (-1 on error, 0 otherwise)
     **/
    virtual int write(byte slave_addr, byte reg_addr, unsigned int length, byte *data) = 0;
};

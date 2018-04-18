/**
    SensIO
    HAL.h

    Definition of the required functions for the hardware abstraction layer.

    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_HAL_H
#define SIDECHANNEL_HAL_H

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
    virtual int read(byte_t slave_addr, byte_t regAddr, unsigned int length, byte_t *data) = 0;

    /**
     * Generic write to sensor
     * @param slaveAddr: Address of the sensor slave
     * @param regAddr: Address of the target register
     * @param length: Number of bytes to write
     * @param data: Data to write
     * @return: (-1 on error, 0 otherwise)
     **/
    virtual int write(byte_t slave_addr, byte_t reg_addr, unsigned int length, byte_t *data) = 0;

    /**
     * Returns bus type
     * @return: (-1 on error, 0 otherwise)
     **/
    //virtual int getBusType() = 0;
};

#endif // SIDECHANNEL_HAL_H
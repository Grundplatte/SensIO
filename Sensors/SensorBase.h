#pragma once

#include "../spdlog/spdlog.h"
#include "../Defines.h"
#include "HAL/HAL.h"

namespace spd = spdlog;

class SensorBase {
public:
    /**
     * Check if the sensor is enabled
     * @return:
     **/
    virtual bool isEnabled() = 0;

    /**
     * Enable the sensor
     * @return: 0
     **/
    virtual int enable() = 0;

    /**
     * Disable the sensor
     * @return: 0
     **/
    virtual int disable() = 0;

    /**
     * Determine which registers are not used and return the addresses as a list
     * @return: list of unused register addresses
     **/
    virtual std::vector<int> getUnusedRegisters() = 0;

    virtual std::vector<int> getSettingRegisters() = 0;

    /**
     * Returns a list of result flags
     * @return list of result flags
     */
    virtual std::vector<bool> getResultFlags() = 0;

    /**
     *
     * @return
     */
    virtual std::vector<int> getResultRegisters() = 0;

    /**
     * Returns the number of "subsensors"
     */
    virtual int getSensorCount() = 0;

    /**
     * Returns the time the sensor need for one measurement cycle
     */
    virtual int getCycleTime() = 0;


    /**
     * Read a value from a sensor register.
     */
    virtual int readRegister(int registerAddress, int size, byte_t &data) = 0;

    /**
     * Write a value to a sensor register
     * @param registerAddress
     * @return
     */
    virtual int writeRegister(int registerAddress, int size, byte_t &data) = 0;


protected:
    std::shared_ptr<spd::logger> _log;
    std::shared_ptr<HAL> _hal;
};
/**
    SensIO
    HTS221.h

    Implementation of the sensor layer for the HTS221 sensor.

    @todo Address translation; wip
    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_HTS221_H
#define SIDECHANNEL_HTS221_H


#include "SensorBase.h"

class HTS221 : public SensorBase{
public:
    bool isEnabled() override;

    int enable() override;

    int disable() override;

    std::vector<int> getUnusedRegisters() override;

    std::vector<int> getSettingRegisters() override;

    std::vector<bool> getResultFlags() override;

    std::vector<int> getResultRegisters() override;

    int getSensorCount() override;

    int getCycleTime() override;

    int readRegister(int registerAddress, int size, byte_t &data) override;

    int writeRegister(int registerAddress, int size, byte_t &data) override;
};


#endif //SIDECHANNEL_HTS221_H

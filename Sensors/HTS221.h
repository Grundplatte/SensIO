/**
    SensIO
    HTS221.h

    Implementation of the sensor layer for the HTS221 sensor.

    @todo Address translation; wip
    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_HTS221_H
#define SIDECHANNEL_HTS221_H

#define HTS221_I2C_ADDR 			0x5F

#define HTS221_WHO		0x0F
#define HTS221_CTRL1		0x20
#define HTS221_CTRL2		0x21
#define HTS221_CTRL3		0x22
#define HTS221_STATUS		0x27
#define HTS221_HUM_OUT_L	0x28
#define HTS221_HUM_OUT_H	0x29
#define HTS221_TMP_OUT_L	0x2A
#define HTS221_TMP_OUT_H	0x2B
#define HTS221_CALIB		0x30 // ...3F

#include "SensorBase.h"

class HTS221 : public SensorBase{
public:
    HTS221(std::shared_ptr<HAL> hal);

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

private:
    std::map<int, int> registerSizeMap;
};


#endif //SIDECHANNEL_HTS221_H

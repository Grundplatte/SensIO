//
// Created by Markus Feldbacher on 16.03.18.
//

#ifndef SIDECHANNEL_LPS25H_H
#define SIDECHANNEL_LPS25H_H

#define LPS25H_I2C_ADDR0                0x5C
#define LPS25H_I2C_ADDR1                0x5D

#define LPS25H_REG_ID               0x0F
#define LPS25H_ID                   0xBD

#define LPS25H_REF_P_XL             0x08
#define LPS25H_REF_P_L              0x09
#define LPS25H_REF_P_H              0x0A
#define LPS25H_WHO                  0x0F
#define LPS25H_REF_CONF             0x10
#define LPS25H_CTRL_REG_1           0x20
#define LPS25H_CTRL_REG_2           0x21
#define LPS25H_CTRL_REG_3           0x22
#define LPS25H_CTRL_REG_4           0x23
#define LPS25H_INT_CFG              0x24
#define LPS25H_INT_SOURCE           0x25
#define LPS25H_STATUS_REG           0x27
#define LPS25H_PRESS_OUT_XL         0x28
#define LPS25H_PRESS_OUT_L          0x29
#define LPS25H_PRESS_OUT_H          0x2A
#define LPS25H_TEMP_OUT_L           0x2B
#define LPS25H_TEMP_OUT_H           0x2C
#define LPS25H_FIFO_CTRL            0x2E
#define LPS25H_FIFO_STATUS          0x2F
#define LPS25H_THS_P_L              0x30
#define LPS25H_THS_P_H              0x31
#define LPS25H_RPDS_L               0x39
#define LPS25H_RPDS_H               0x3A

#include "SensorBase.h"

class LPS25H : public SensorBase {
public:
    LPS25H(std::shared_ptr<HAL> hal);

    bool isEnabled() override;
    int enable() override;
    int disable() override;
    std::vector<int> getUnusedRegisters() override;
    std::vector<bool> getResultFlags() override;
    std::vector<int> getResultRegisters() override;
    int getSensorCount() override;
    int getCycleTime() override;
    int readRegister(int registerAddress, int size, byte_t &data) override;
    int writeRegister(int registerAddress, int size, byte_t &data) override;


private:
    std::map<int, int> registerSizeMap;
};


#endif //SIDECHANNEL_LPS25H_H

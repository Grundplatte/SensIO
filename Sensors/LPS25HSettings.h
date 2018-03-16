#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "Sensor.h"

#define I2C_LPS25H_ADDR0                0x5C
#define I2C_LPS25H_ADDR1                0x5D
#define I2C_LPS25H_REG_ID               0x0F
#define I2C_LPS25H_ID                   0xBD

#define I2C_LPS25H_REF_P_XL             0x08
#define I2C_LPS25H_REF_P_L              0x09
#define I2C_LPS25H_REF_P_H              0x0A
#define I2C_LPS25H_WHO                  0x0F
#define I2C_LPS25H_REF_CONF             0x10
#define I2C_LPS25H_CTRL_REG_1           0x20
#define I2C_LPS25H_CTRL_REG_2           0x21
#define I2C_LPS25H_CTRL_REG_3           0x22
#define I2C_LPS25H_CTRL_REG_4           0x23
#define I2C_LPS25H_INT_CFG              0x24
#define I2C_LPS25H_INT_SOURCE           0x25
#define I2C_LPS25H_STATUS_REG           0x27
#define I2C_LPS25H_PRESS_OUT_XL         0x28
#define I2C_LPS25H_PRESS_OUT_L          0x29
#define I2C_LPS25H_PRESS_OUT_H          0x2A
#define I2C_LPS25H_TEMP_OUT_L           0x2B
#define I2C_LPS25H_TEMP_OUT_H           0x2C
#define I2C_LPS25H_FIFO_CTRL            0x2E
#define I2C_LPS25H_FIFO_STATUS          0x2F
#define I2C_LPS25H_THS_P_L              0x30
#define I2C_LPS25H_THS_P_H              0x31
#define I2C_LPS25H_RPDS_L               0x39
#define I2C_LPS25H_RPDS_H               0x3A


class LPS25HSettings : public Sensor {
public:
    LPS25HSettings(std::shared_ptr<HAL> hal);

    int isActive() override;

    int toggleOnOff(bit_t on_off) override;

    int waitForSensReady() override;

    int sendBit(bit_t bit) override;

    int sendByte(unsigned char inbyte) override;

    int readBit(bool timeout, int long_timeout) override;

    int readByte() override;

    int sendReset() override;

    int supportsBytes() override;

    void wait(int cycle_count) override;

    // not used
    //int send(byte *data, int length);

    //int receive(byte *data);

    //int detectUsage();

private:
    unsigned char _ref_value = 0;
    bool _last_bit = 0;
};



#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "HAL/I2C_HAL.h"
#include "Sensor.h"

// HTS221
#define I2C_TEMP_ADDR 			0x5F
#define I2C_TEMP_REG_WHO		0x0F
#define I2C_TEMP_REG_CTRL1		0x20
#define I2C_TEMP_REG_CTRL2		0x21
#define I2C_TEMP_REG_CTRL3		0x22
#define I2C_TEMP_REG_STATUS		0x27
#define I2C_TEMP_REG_HUM_OUT_L	0x28
#define I2C_TEMP_REG_HUM_OUT_H	0x29
#define I2C_TEMP_REG_TMP_OUT_L	0x2A
#define I2C_TEMP_REG_TMP_OUT_H	0x2B
#define I2C_TEMP_REG_CALIB		0x30 // ...3F

class HTS221 : public Sensor {
public:
    HTS221();

    ~HTS221();

    int isActive() override;

    int toggleOnOff(bit_t onOff) override;

    int waitForSensReady() override;

    int sendBit(bit_t bit) override;

    int tryReadBit() override;

    int sendReset() override;

    // not used
    int send(byte *data, int length);

    int receive(byte *data);

    int detectUsage();

private:

    I2C_HAL *m_i2c;

    int isSensReady(uint8_t status);
    int isTempReady(uint8_t status);
    int isHumReady(uint8_t status);
    int getStatus(uint8_t *status);
};



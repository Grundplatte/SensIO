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

    // TODO: export to midlayer
    int waitForSensReady();
    int sendBit(uint8_t bit);
    int tryReadBit();
    int sendReset();

private:
    I2C_HAL *m_i2c;

    int isSensReady(uint8_t status);
    int isTempReady(uint8_t status);
    int isHumReady(uint8_t status);
    int getStatus(uint8_t *status);



// TODO: old stuff
    int detectUsage();
    int send(byte *data, int length);
    int receive(byte *data);
};


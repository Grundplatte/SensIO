#pragma once

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "linux/i2c.h"
#include "linux/i2c-dev.h"
#include "../../Defines.h"
#include "HAL.h"

#define I2C_INTERFACE "/dev/i2c-1"
#define CONF 3
#define MAX_READ_LEN 4
#define MAX_WRITE_LEN 2


class I2C_HAL : HAL {
public:

    int read(byte slaveAddr, byte regAddr, unsigned int length, byte *data) override ;
    int write(byte slaveAddr, byte regAddr, unsigned int length, byte *data) override ;

    I2C_HAL();
    ~I2C_HAL();
private:
    int m_I2C;
};

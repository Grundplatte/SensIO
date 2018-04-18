/**
    SensIO
    I2C_HAL.h

    Implementation of I2C HAL. This was tested on a raspberry pi, which has two I2C busses. The bus can be
    changed by altering the I2C_INTERFACE definition.

    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_I2C_HAL_H
#define SIDECHANNEL_I2C_HAL_H

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
//#define I2C_INTERFACE "/dev/i2c-0"

#define MAX_READ_LEN 4
#define MAX_WRITE_LEN 2


class I2C_HAL : public HAL {
public:

    int read(byte_t slave_addr, byte_t reg_addr, unsigned int length, byte_t *data) override;

    int write(byte_t slave_addr, byte_t reg_addr, unsigned int length, byte_t *data) override;

    I2C_HAL();
    ~I2C_HAL();

private:
    int _i2c;
};

#endif //SIDECHANNEL_I2C_HAL_H
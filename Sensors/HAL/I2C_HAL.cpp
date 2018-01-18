#include "I2C_HAL.h"

/*
 * I2C settings
 */

I2C_HAL::I2C_HAL(){
    _i2c = open(I2C_INTERFACE, O_RDWR);
    if (_i2c < 0) {
        exit(EXIT_FAILURE);
    }
};
 
I2C_HAL::~I2C_HAL()
{
    if (_i2c != 0) {
        close(_i2c);
	}
}

int I2C_HAL::read(byte slave_addr, byte reg_addr, unsigned int length, byte *data)
{
	int result;
    byte rx_buff[MAX_READ_LEN];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data ioctl_data{};

    msgs[0].addr = slave_addr;
	msgs[0].flags = 0x00;
	msgs[0].len = 1;
    msgs[0].buf = &reg_addr;

    msgs[1].addr = slave_addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = length;
    msgs[1].buf = rx_buff;
	
	ioctl_data.msgs = msgs;
	ioctl_data.nmsgs = 2;

    result = ioctl(_i2c, I2C_RDWR, &ioctl_data);
    memcpy(data, rx_buff, length);
	return result;
}

int I2C_HAL::write(byte slave_addr, byte reg_addr, unsigned int length, byte *data)
{
	int result;
    byte tx_buff[MAX_WRITE_LEN];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data ioctl_data{};

    memcpy(tx_buff, data, length);

    msgs[0].addr = slave_addr;
	msgs[0].flags = 0x00;
	msgs[0].len = 1 + length;
	msgs[0].buf = (byte *) malloc(1 + length);

    msgs[0].buf[0] = reg_addr;
	memcpy(msgs[0].buf + 1, data, length);
	
	ioctl_data.msgs = msgs;
	ioctl_data.nmsgs = 1;

    result = ioctl(_i2c, I2C_RDWR, &ioctl_data);

	free(msgs[0].buf);

	return result;
}

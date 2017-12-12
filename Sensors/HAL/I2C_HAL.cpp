#include "I2C_HAL.h"

/*
 * I2C settings
 */

I2C_HAL::I2C_HAL(){
    m_I2C = open(I2C_INTERFACE, O_RDWR);
    if(m_I2C < 0){
        exit(EXIT_FAILURE);
    }
};
 
I2C_HAL::~I2C_HAL()
{
	if(m_I2C != 0)
	{
		close(m_I2C);
	}
}

int I2C_HAL::read(byte slaveAddr, byte regAddr, unsigned int length, byte *data)
{
	int result;
    byte rxBuff[MAX_READ_LEN];
    struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
    
    msgs[0].addr = slaveAddr;
	msgs[0].flags = 0x00;
	msgs[0].len = 1;
	msgs[0].buf = &regAddr;
	
	msgs[1].addr = slaveAddr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = length;
	msgs[1].buf = rxBuff;
	
	ioctl_data.msgs = msgs;
	ioctl_data.nmsgs = 2;

	result = ioctl(m_I2C, I2C_RDWR, &ioctl_data);
	memcpy(data, rxBuff, length);
	return result;
}

int I2C_HAL::write(byte slaveAddr, byte regAddr, unsigned int length, byte *data)
{
	int result;
    byte txBuff[MAX_WRITE_LEN];
    struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
    
    memcpy(txBuff, data, length);
    
    msgs[0].addr = slaveAddr;
	msgs[0].flags = 0x00;
	msgs[0].len = 1 + length;
	msgs[0].buf = (byte *) malloc(1 + length);

	msgs[0].buf[0] = regAddr;
	memcpy(msgs[0].buf + 1, data, length);

	//msgs[1].addr = slaveAddr;
	//msgs[1].flags = 0x00;
	//msgs[1].len = length;
	//msgs[1].buf = txBuff;
	
	ioctl_data.msgs = msgs;
	ioctl_data.nmsgs = 1;

	result = ioctl(m_I2C, I2C_RDWR, &ioctl_data);

	free(msgs[0].buf);

	return result;
}

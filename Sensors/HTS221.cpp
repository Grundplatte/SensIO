#include "HTS221.h"

HTS221::HTS221() {
    m_i2c = new I2C_HAL();
}

int HTS221::isSensReady(byte status)
{
	return (status & 0x03) == 0x03;
}
int HTS221::isTempReady(byte status)
{
	return status & 0x01;
}
int HTS221::isHumReady(byte status)
{
	return (status & 0x02)>>1;
}

int HTS221::getStatus(byte *status)
{
	return m_i2c->read(I2C_TEMP_ADDR, I2C_TEMP_REG_STATUS, 1, status);
}

int HTS221::waitForSensReady()
{
	byte status;
	unsigned int i;
	int result;
	
	for(i=0; i<CONF; i++){
		do {
			result = getStatus(&status);
		} while(!isSensReady(status));
	}
	return 0;
}

// read temp = 0; read hum = 1;
int HTS221::sendBit(byte bit)
{
    byte data[2];
    byte status;
		
	//printf("[D] sendBit %x\n", bit?1:0);
	
	// 0
	if(bit == 0x00){
		// read tmpout register with autoincrement address
		m_i2c->read(I2C_TEMP_ADDR, I2C_TEMP_REG_TMP_OUT_L + 0x80, 2, data);
	}
	else{
		// 1
		// read humout
        m_i2c->read(I2C_TEMP_ADDR, I2C_TEMP_REG_HUM_OUT_L + 0x80, 2, data);
	}
	
	return 0;
}

int HTS221::sendReset()
{
    byte data[4];
    m_i2c->read(I2C_TEMP_ADDR, I2C_TEMP_REG_HUM_OUT_L + 0x80, 4, data);
	
	return 0;
}

/*
 * Sending functions
 */

// send raw data
int HTS221::send(byte *data, int length)
{
	unsigned int l;
	int i;
    byte bit;
	struct timespec req, rem;
	
	bit = 0x01;
	req.tv_sec = 0;
	req.tv_nsec = 10000000; // 10ms

	for(l=0; l<length; l++){
		for(i=7; i>=0; i--) {	
			// wait until the sensor is ready
			waitForSensReady();
				
			sendBit(data[l] & (bit << i));
			
			nanosleep(&req, &rem);
		}
		printf("[D] send: sent 0x%02x\n", data[l]);
	}
}

int HTS221::tryReadBit()
{
    byte status;
	unsigned int i;
	int result;
	result = getStatus(&status);
		
	// nothing read
	if(isSensReady(status)){
		return -1;
	}
	// temp was read => 0
	if(isHumReady(status) && !isTempReady(status)){
		// reread a few times to confirm
		for(i=0; i<CONF; i++){
			result = getStatus(&status);
			if(!isHumReady(status)){
				printf("[E] tryReadBit: someone interfered?\n");
				return -2;
			}
		}
		return 0;
	}
	// hum was read => 1
	if(isTempReady(status) && !isHumReady(status)){
		// reread a few times to confirm
		for(i=0; i<CONF; i++){
			result = getStatus(&status);
			if(!isTempReady(status)){
				printf("[E] tryReadBit: someone interfered?\n");
				return -2;
			}
		}
		return 1;
	}
	// error
	printf("[E] tryReadBit: status corrupted:  0x%x\n", status);
	return -2;
}

/*
 * Receiving function
 */

int HTS221::receive(uint8_t *data)
{
	uint32_t l;
	int bit, i;
	
	for(l=0; l<sizeof(data); l++){
		data[l] = 0;
		
		for(i=7; i>=0; i--) {		
			waitForSensReady();
			// wait until someone accesses the sensor results
			do {
				bit = tryReadBit();
				//printf("[D] receive: bit = 0x%02x\n", bit);
			} while(bit < 0);
			
			if(bit){
				data[l] |= (1 << i);
				//printf("[D] receive: received 1\n", data[l]);
			}
			else{
				//printf("[D] receive: received 0\n", data[l]);
			}
		}
		printf("[D] receive: received 0x%02x\n", data[l]);
	}
}

// try to detect access patterns, to avoid interference
// TODO: timeout after xx sec/min
// TODO: try to find pattern
// TODO: adjust access times
int HTS221::detectUsage()
{
	int result;
	uint8_t status;
	uint32_t i;
	struct timespec *time1p, *time2p, *temp, time1, time2;
	long int diffs, diffns;
	
	time1p = &time1;
	time2p = &time2;
	
	for(i=0; i<10; i++){
		// wait for ready
		do {
			result = getStatus(&status);
		} while(!isSensReady(status));
		
		// wait for someone to read sensor results
		do {
			result = getStatus(&status);
		} while(isSensReady(status));
		
		clock_gettime(CLOCK_REALTIME, time1p);
		
		if(i != 0) {
			diffs = time1p->tv_sec - time2p->tv_sec;
			diffns = time1p->tv_nsec - time2p->tv_nsec;
			
			// rollover
			if(diffns < 0) {
				diffs -= 1;
				diffns += 1000000000;
			}
			
			printf("[D] Sensor used after %2i.%3is\n", diffs, diffns/1000000L);
		}
		
		// switch time pointers
		temp = time1p;
		time1p = time2p;
		time2p = temp;
	}
}

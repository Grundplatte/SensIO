#include "HTS221.h"

HTS221::HTS221() {
    std::shared_ptr<spdlog::logger> log;
    log = spd::get("HTS221");

    if (log)
        m_log = log;
    else
        m_log = spd::stdout_color_mt("HTS221");

    m_i2c = new I2C_HAL();
}

HTS221::~HTS221() {}

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
    int i;

    m_log->debug("Waiting for sensor...");

	for(i=0; i<CONF; i++){
		do {
            getStatus(&status);

            // TODO: if this takes more then a few cycles, check if sensor is active
		} while(!isSensReady(status));
	}
	return 0;
}

// read temp = 0; read hum = 1;
int HTS221::sendBit(bit bit)
{
    byte data[2];

    // 1
    if (bit) {
		// read tmpout register with autoincrement address
        m_log->debug("Send bit 1");
        m_i2c->read(I2C_TEMP_ADDR, I2C_TEMP_REG_HUM_OUT_L + 0x80, 2, data);
    } else {
        // 0
        // read humout
        m_log->debug("Send bit 0");
        m_i2c->read(I2C_TEMP_ADDR, I2C_TEMP_REG_TMP_OUT_L + 0x80, 2, data);
    }
	
	return 0;
}

int HTS221::sendReset()
{
    byte data[4];

    // read both (tmpout + humout) in one go
    m_i2c->read(I2C_TEMP_ADDR, I2C_TEMP_REG_HUM_OUT_L + 0x80, 4, data);

    return 0;
}

/*
 * Sending functions
 */

// send raw data
int HTS221::send(byte *data, int length)
{
    int i, l;
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
	}
}

int HTS221::tryReadBit()
{
    byte status;
    int i;

    getStatus(&status);

	// nothing read
	if(isSensReady(status)){
		return -1;
	}
	// temp was read => 0
	if(isHumReady(status) && !isTempReady(status)){
        // FIXME: wait a few ms, then read again to put less pressure on the bus
		// reread a few times to confirm
		for(i=0; i<CONF; i++){
            getStatus(&status);
			if(!isHumReady(status)){
                m_log->warn("Someone interfered?\n");
			}
		}

        m_log->debug("Received bit 0");
		return 0;
	}
	// hum was read => 1
	if(isTempReady(status) && !isHumReady(status)){
        // FIXME: wait a few ms, then read again to put less pressure on the bus
		// reread a few times to confirm
		for(i=0; i<CONF; i++){
            getStatus(&status);
			if(!isTempReady(status)){
                m_log->warn("Someone interfered?\n");
			}
		}

        m_log->debug("Received bit 1");
		return 1;
	}
	// error
    m_log->warn("Status corrupted:  0x{0:2x}\n", status);
	return -2;
}

/*
 * Receiving function
 */

int HTS221::receive(uint8_t *data)
{
    int bit, i, l;
	
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
        m_log->debug("receive: received 0x{0:2x}\n", data[l]);
	}
}

// try to detect access patterns, to avoid interference
// TODO: timeout after xx sec/min
// TODO: try to find pattern
// TODO: adjust access times
int HTS221::detectUsage()
{
    byte status;
    int i;
	struct timespec *time1p, *time2p, *temp, time1, time2;
	long int diffs, diffns;
	
	time1p = &time1;
	time2p = &time2;
	
	for(i=0; i<10; i++){
		// wait for ready
		do {
            getStatus(&status);
		} while(!isSensReady(status));
		
		// wait for someone to read sensor results
		do {
            getStatus(&status);
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

            m_log->debug("Sensor used after {0:2}.{1:3}s", diffs, diffns / 1000000L);
		}
		
		// switch time pointers
		temp = time1p;
		time1p = time2p;
		time2p = temp;
	}
}

int HTS221::isActive() {
    byte data;

    // read both (tmpout + humout) in one go
    m_i2c->read(I2C_TEMP_ADDR, I2C_TEMP_REG_CTRL1, 1, &data);

    m_log->debug("Sensor HTS221 data: 0x{0:2x}", data);

    if (data & 0x80) {
        // active
        if ((data & 0x07) != 0x07) {
            data = 0x87;
            m_i2c->write(I2C_TEMP_ADDR, I2C_TEMP_REG_CTRL1, 1, &data);
        }

        return 1;
    }

    return 0;
}

int HTS221::toggleOnOff(bit
onOff) {
    byte data;

    m_log->debug("Sensor HTS221 toggle: {0}", onOff);

    if (onOff)
        data = 0x87;
    else
        data = 0x07;

    m_i2c->write(I2C_TEMP_ADDR, I2C_TEMP_REG_CTRL1, 1, &data);

    return 0;
}

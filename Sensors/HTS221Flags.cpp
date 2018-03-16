#include "HTS221Flags.h"

HTS221Flags::HTS221Flags(std::shared_ptr<HAL> hal) {
    std::shared_ptr<spdlog::logger> log = spd::get("HTS221Flags");
    _log = log ? log : spd::stdout_color_mt("HTS221Flags");
    _hal = hal;
}

int HTS221Flags::isSensReady(byte_t status)
{
	return (status & 0x03) == 0x03;
}
int HTS221Flags::isTempReady(byte_t status)
{
	return status & 0x01;
}
int HTS221Flags::isHumReady(byte_t status)
{
	return (status & 0x02)>>1;
}

int HTS221Flags::getStatus(byte_t *status)
{
    return _hal->read(I2C_TEMP_ADDR, I2C_TEMP_REG_STATUS, 1, status);
}

int HTS221Flags::waitForSensReady()
{
	byte_t status;

    _log->trace("Waiting for sensor...");

    //for (int i = 0; i < CONF; i++) {
		do {
            getStatus(&status);
            // TODO: if this takes more then a few cycles, check if sensor is active
		} while(!isSensReady(status));
    //}

    _log->trace("Ready...");
	return 0;
}

// read temp = 0; read hum = 1;
int HTS221Flags::sendBit(bit_t bit)
{
    byte_t data[2];
    struct timespec req{}, rem{};
    req.tv_sec = 0;
    req.tv_nsec = WRITE_DELAY;

    waitForSensReady();
    nanosleep(&req, &rem);

    // 1
    if (bit) {
		// read tmpout register with autoincrement address
        _log->trace("Send bit 1");
        _hal->read(I2C_TEMP_ADDR, I2C_TEMP_REG_HUM_OUT_L + 0x80, 2, data);
    } else {
        // 0
        // read humout
        _log->trace("Send bit 0");
        _hal->read(I2C_TEMP_ADDR, I2C_TEMP_REG_TMP_OUT_L + 0x80, 2, data);
    }

    return 0;
}

int HTS221Flags::isActive() {
    byte_t data;

    // read both (tmpout + humout) in one go
    _hal->read(I2C_TEMP_ADDR, I2C_TEMP_REG_CTRL1, 1, &data);

    _log->trace("Sensor HTS221Flags data: 0x{0:2x}", data);

    if (data & 0x80) {
        // active
        if ((data & 0x07) != 0x07) {
            data = 0x87;
            _hal->write(I2C_TEMP_ADDR, I2C_TEMP_REG_CTRL1, 1, &data);
        }

        return 1;
    }

    return 0;
}

int HTS221Flags::toggleOnOff(bit_t on_off) {
    byte_t data;

    _log->trace("Sensor HTS221Flags toggle: {0}", on_off);

    if (on_off)
        data = 0x87;
    else
        data = 0x07;

    _hal->write(I2C_TEMP_ADDR, I2C_TEMP_REG_CTRL1, 1, &data);

    return 0;
}

int HTS221Flags::sendReset()
{
    byte_t data[4];

    // read both (tmpout + humout) in one go
    _hal->read(I2C_TEMP_ADDR, I2C_TEMP_REG_HUM_OUT_L + 0x80, 4, data);

    return 0;
}

/*
 * Sending functions
 */

int HTS221Flags::tryReadBit()
{
    byte_t status;
    getStatus(&status);

	// nothing read
	if(isSensReady(status)){
		return -1;
	}
	// temp was read => 0
	if(isHumReady(status) && !isTempReady(status)){
        _log->trace("Received bit 0");
		return 0;
	}
	// hum was read => 1
	if(isTempReady(status) && !isHumReady(status)){
        _log->trace("Received bit 1");
		return 1;
	}
	// error
    _log->warn("Status corrupted:  0x{0:2x}\n", status);
	return -2;
}

int HTS221Flags::supportsBytes() {
    return 0;
}

int HTS221Flags::readByte() {
    _log->error("Attack does not support bytes.");
    exit(EXIT_FAILURE);
}

int HTS221Flags::sendByte(unsigned char byte) {
    _log->error("Attack does not support bytes.");
    exit(EXIT_FAILURE);
}

int HTS221Flags::readBit(bool timeout, int long_timeout) {
    struct timespec start{}, stop{};
    double accum;
    int bit;
    int LONG_DELAY = long_timeout * CYCLE_DELAY;

    waitForSensReady();
    clock_gettime(CLOCK_REALTIME, &start);
    // wait until someone accesses the sensor results
    do {
        bit = tryReadBit();

        clock_gettime(CLOCK_REALTIME, &stop);
        accum = stop.tv_nsec - start.tv_nsec;
        // rollover
        if (accum < 0) {
            accum += 1000000000;
        }

        // timeout > delay ms (only relevant in transmission has already started)
        if (timeout && accum > CYCLE_DELAY) {
            _log->warn("[D] Timeout while receiving");
            return TIMEOUT_WHILE_RECEIVING;
        }else if(long_timeout && accum > LONG_DELAY){
            _log->warn("[D] Timeout while waiting");
            return TIMEOUT_WHILE_WAITING;
        }
    } while (bit < 0);

    return bit;
}

void HTS221Flags::wait(int cycle_count) {
    struct timespec req{}, rem{};
    req.tv_sec = 0;
    req.tv_nsec = (__syscall_slong_t) CYCLE_DELAY * cycle_count; // 80ms per cycle

    nanosleep(&req, &rem);
}

/*
// send raw data
int HTS221Flags::send(byte *data, int length)
{
    byte bit;
    bit = 0x01;

    struct timespec req{}, rem{};
    req.tv_sec = 0;
    req.tv_nsec = 10000000; // 10ms

    for (int l = 0; l < length; l++) {
        for (int i = 7; i >= 0; i--) {
            // wait until the sensor is ready
            waitForSensReady();

            sendBit(data[l] & (bit << i));

            nanosleep(&req, &rem);
        }
    }
}


int HTS221Flags::receive(uint8_t *data)
{
    int bit;

    for (int l = 0; l < sizeof(data); l++) {
		data[l] = 0;

        for (int i = 7; i >= 0; i--) {
			waitForSensReady();
			// wait until someone accesses the sensor results
			do {
				bit = tryReadBit();
			} while(bit < 0);

			if(bit){
				data[l] |= (1 << i);
			}
			else{
			}
		}
        _log->debug("receive: received 0x{0:2x}\n", data[l]);
	}
}

// try to detect access patterns, to avoid interference
// TODO: timeout after xx sec/min
// TODO: try to find pattern
// TODO: adjust access times
int HTS221Flags::detectUsage()
{
    byte status;
    struct timespec *time1p, *time2p, *temp, time1{}, time2{};
	long int diffs, diffns;

	time1p = &time1;
	time2p = &time2;

    for (int i = 0; i < 10; i++) {
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

            _log->debug("Sensor used after {0:2}.{1:3}s", diffs, diffns / 1000000L);
		}

		// switch time pointers
		temp = time1p;
		time1p = time2p;
		time2p = temp;
	}
}
*/
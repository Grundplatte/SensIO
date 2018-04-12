#include "LPS25HUnused.h"
#include "../TestBed.h"

LPS25HUnused::LPS25HUnused(std::shared_ptr<HAL> hal) {
    std::shared_ptr<spdlog::logger> log = spd::get("LPS25HUnused");
    _log = log ? log : spd::stdout_color_mt("LPS25HUnused");
    _hal = hal;

    // isActive not needed
    _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, &_last_byte);
    _log->trace("Initial state: {0:x}", _last_byte);
    if(_last_byte != 0xFF){
        unsigned char data = 0xFF;
        _hal->write(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, &data);

        _last_byte = 0xFF;
    }
    else{
        _transmission = true;
    }
}

int LPS25HUnused::waitForSensReady()
{
    // sensor is allways ready
    _log->trace("Waiting for sensor...");
    _log->trace("Ready...");
	return 0;
}

int LPS25HUnused::isActive() {
    byte_t data;

    // read both (tmpout + humout) in one go
    _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_CTRL_REG_1, 1, &data);
    _log->trace("Sensor HTS221Flags data: 0x{0:2x}", data);

    if (data & 0x80) {
        // active
        return 1;
    }

    return 0;
}

int LPS25HUnused::toggleOnOff(bit_t on_off) {
    byte_t data;

    _log->trace("Sensor HTS221Flags toggle: {0}", on_off);
    _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_CTRL_REG_1, 1, &data);

    if (on_off)
        data |= 0x80;
    else
        data &= 0x7F;

    _hal->write(I2C_LPS25H_ADDR0, I2C_LPS25H_CTRL_REG_1, 1, &data);

    return 0;
}

/*
 * Sending functions
 */

int LPS25HUnused::readBit(bool timeout, int long_timeout)
{
    _log->error("Not supported");
    exit(EXIT_FAILURE);
	//return readByte(false); //FIXME: handle sender/receiver status
}

int LPS25HUnused::readByte() {
    // check if new data is available. if there is new data, save it and write the inverted value to the sensor
    byte_t data[1];
    struct timespec req{}, rem{};
    req.tv_sec = 1;
    req.tv_nsec = 0; // 1s per cycle

    _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, data);

    if(!_transmission){
        // init state, check every second for request
        while (memcmp(&data, &_last_byte, 1) == 0){
            nanosleep(&req, &rem);
            _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, data);
        }

        _transmission = true;
    }
    else {
        // normal receive
        while(memcmp(&data, &_last_byte, 1) == 0){
            // nothing changed
            _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, data);
            //TODO: add timeout
        }

        _log->trace("readByte: {0:x}  (last_byte: {1:x})", data[0], _last_byte);
    }

    return data[0];
}

// read temp = 0; read hum = 1;
int LPS25HUnused::sendBit(bit_t bit)
{
    return bit ? sendByte(0x01) : sendByte(0x00);
}

int LPS25HUnused::sendByte(unsigned char inbyte) {
    byte_t data[1];

    /* TODO: needed?
    _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, data);

    while(_transmission && memcmp(data, &_last_byte, 1) == 0){
        // nothing changed
        _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, data);
        //TODO: add timeout
    }*/

    // sender
    if(TestBed::TYPE){
        inbyte |= 0x80;
    }

    _log->trace("sendByte: {0:x} (_last_byte {1:x})", inbyte, _last_byte);
    _hal->write(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, &inbyte);
    _transmission = true;
    _last_byte = inbyte;

    return 0;
}

int LPS25HUnused::sendReset()
{
    _log->warn("sendReset: Not implemented!");
    return 0;
}

int LPS25HUnused::supportsBytes() {
    return 1;
}

void LPS25HUnused::wait(int cycle_count) {
    // not needed
}

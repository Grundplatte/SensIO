#include "LPS25HUnused.h"

LPS25HUnused::LPS25HUnused(std::shared_ptr<HAL> hal) {
    std::shared_ptr<spdlog::logger> log = spd::get("LPS25HUnused");
    _log = log ? log : spd::stdout_color_mt("LPS25HUnused");
    _hal = hal;

    // isActive not needed
 //   _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, &_last_byte);
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
	return readByte();
}

int LPS25HUnused::readByte() {
    // check if new data is available. if there is new data, save it and write the inverted value to the sensor
    byte_t data[1];

    _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, data);

    while(_last_byte_valid && memcmp(&data, &_last_byte, 1) == 0){
        // nothing changed
        _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, data);
        //TODO: add timeout
    }

    // invert => checksum?
    data[0] = ~data[0];

    _hal->write(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, data);
    _last_byte_valid = true;
    _last_byte = data[0];
}

// read temp = 0; read hum = 1;
int LPS25HUnused::sendBit(bit_t bit)
{
    return bit ? sendByte(0x01) : sendByte(0x00);
}

int LPS25HUnused::sendByte(unsigned char inbyte) {
    byte_t data[1];

    _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, data);

    while(_last_byte_valid && memcmp(data, &_last_byte, 1) == 0){
        // nothing changed
        _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, data);
        //TODO: add timeout
    }

    _hal->write(I2C_LPS25H_ADDR0, I2C_LPS25H_THS_P_L, 1, &inbyte);
    _last_byte_valid = true;
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
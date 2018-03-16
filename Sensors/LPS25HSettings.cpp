#include "LPS25HSettings.h"
#include "../TestBed.h"

LPS25HSettings::LPS25HSettings(std::shared_ptr<HAL> hal) {
    std::shared_ptr<spdlog::logger> log = spd::get("LPS25HUnused");
    _log = log ? log : spd::stdout_color_mt("LPS25HUnused");
    _hal = hal;

    // isActive not needed
    _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_REF_P_XL, 1, &_ref_value);
    _log->trace("Initial state: {0:x}", _ref_value);
    if(_ref_value != (_ref_value & 0xFC)){
        _ref_value &= 0xFC;
        _hal->write(I2C_LPS25H_ADDR0, I2C_LPS25H_REF_P_XL, 1, &_ref_value);
    }

    _last_bit = 0;
}

int LPS25HSettings::waitForSensReady()
{
    // sensor is allways ready
    _log->trace("Waiting for sensor...");
    _log->trace("Ready...");
	return 0;
}

int LPS25HSettings::isActive() {
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

int LPS25HSettings::toggleOnOff(bit_t on_off) {
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

int LPS25HSettings::readBit(bool timeout, int long_timeout)
{
    // TODO: implement
    unsigned char data[1];
    unsigned char ack[1];
    bool new_bit;

    do {
        _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_REF_P_XL, 1, data);

        new_bit = (bool) (data[0] & 0x02);

    } while(new_bit == _last_bit);

    // new data available
    _last_bit = new_bit;
    _log->trace("Received bit {0:x} ({1:x})", data[0] & 0x01, data[0] & 0x02);

    // write ack
    ack[0] = _ref_value;
    if(!_last_bit){
        ack[0] |= 0x02;
    }
    _hal->write(I2C_LPS25H_ADDR0, I2C_LPS25H_REF_P_XL, 1, ack);
    _last_bit = !_last_bit;
    _log->trace("Sent ACK ({0:x})", ack[0] & 0x02);

    return data[0] & 0x01;
}

int LPS25HSettings::readByte() {
    _log->error("Not supported");
    exit(EXIT_FAILURE);
    //return readByte(false); //FIXME: handle sender/receiver status
}

// read temp = 0; read hum = 1;
int LPS25HSettings::sendBit(bit_t bit)
{
    // TODO: implement
    unsigned char data[1];
    data[0] = _ref_value;

    if(bit != 0){
        data[0] |= 0x01;
    }

    if(!_last_bit){
        data[0] |= 0x02;
    }

    _log->trace("Sent bit {0:x} ({1:x})", data[0] & 0x01, data[0] & 0x02);
    _hal->write(I2C_LPS25H_ADDR0, I2C_LPS25H_REF_P_XL, 1, data);
    _last_bit = !_last_bit;

    // get ack
    bool new_bit;
    do {
        _hal->read(I2C_LPS25H_ADDR0, I2C_LPS25H_REF_P_XL, 1, data);

        new_bit = (bool) (data[0] & 0x02);

    } while(new_bit == _last_bit);

    // new data available
    _last_bit = new_bit;
    _log->trace("Received ACK ({0:x})", data[0] & 0x02);
}

int LPS25HSettings::sendByte(unsigned char inbyte) {
    _log->error("Not supported");
    exit(EXIT_FAILURE);
    //return readByte(false); //FIXME: handle sender/receiver status
}

int LPS25HSettings::sendReset()
{
    _log->warn("sendReset: Not implemented!");
    return 0;
}

int LPS25HSettings::supportsBytes() {
    return 0;
}

void LPS25HSettings::wait(int cycle_count) {
    // not needed
}

/**
    SensIO
    LPS25H.cpp

    Implementation of the sensor layer for the LPS25H.

    @todo Address translation; wip
    @author Markus Feldbacher
*/

#include "LPS25H.h"

LPS25H::LPS25H(std::shared_ptr<HAL> hal) {
    std::shared_ptr<spdlog::logger> log = spd::get("LPS25H");
    _log = log ? log : spd::stdout_color_mt("LPS25H");
    _hal = hal;

    // init register size map with registers >1byte
    registerSizeMap.insert( std::pair<int, int>(LPS25H_REF_P_XL, 3));
    registerSizeMap.insert( std::pair<int, int>(LPS25H_CTRL_REG_1, 4));
    registerSizeMap.insert( std::pair<int, int>(LPS25H_PRESS_OUT_XL, 3));
    registerSizeMap.insert( std::pair<int, int>(LPS25H_TEMP_OUT_L, 2));
    registerSizeMap.insert( std::pair<int, int>(LPS25H_THS_P_L, 2));
    registerSizeMap.insert( std::pair<int, int>(LPS25H_RPDS_L, 2));
}

bool LPS25H::isEnabled() {
    byte_t data;

    _hal->read(LPS25H_I2C_ADDR, LPS25H_CTRL_REG_1, 1, &data);

    return (bool) (data & 0x80);
}

int LPS25H::enable() {
    byte_t data;

    _log->trace("Enabling Sensor.");
    _hal->read(LPS25H_I2C_ADDR, LPS25H_CTRL_REG_1, 1, &data);

    data |= 0x80;

    _hal->write(LPS25H_I2C_ADDR, LPS25H_CTRL_REG_1, 1, &data);

    return 0;
}

int LPS25H::disable() {
    byte_t data;

    _log->trace("Disabling Sensor.");
    _hal->read(LPS25H_I2C_ADDR, LPS25H_CTRL_REG_1, 1, &data);

    data &= 0x7F;

    _hal->write(LPS25H_I2C_ADDR, LPS25H_CTRL_REG_1, 1, &data);

    return 0;
}

std::vector<int> LPS25H::getUnusedRegisters() {
    byte_t  data;
    std::vector<int> output;

    // check interrupt enabled bit CTRL_1
    _hal->read(LPS25H_I2C_ADDR, LPS25H_CTRL_REG_1, 1, &data);
    if(data & 0x08){
        // check interrupt configuration CTRL_3
        _hal->read(LPS25H_I2C_ADDR, LPS25H_CTRL_REG_3, 1, &data);
        if(data & 0x03){
            // Threshold in use
            return output;
        }
    }

    // Threshold register
    int tmp = LPS25H_THS_P_H;
    output.push_back(tmp);
    tmp = LPS25H_THS_P_L;
    output.push_back(tmp);

    return output;
}

std::vector<int> LPS25H::getResultRegisters() {
    std::vector<int> resultRegisters;

    // TODO: check if reading the first register is sufficient
    resultRegisters.push_back(LPS25H_PRESS_OUT_H); // datasheet specifies high registers must be read to reset flag
    resultRegisters.push_back(LPS25H_TEMP_OUT_H);

    return resultRegisters;
}

int LPS25H::getSensorCount() {
    return 2;
}

int LPS25H::getCycleTime() {
    return 40; //ms
}

int LPS25H::readRegister(int registerAddress, int size, byte_t &data) {
    // some size checks
    if(size < 0){
        _log->error("Cant read registers with negative size!");
        return -1;
    }

    if(size > 1){
        auto iter = registerSizeMap.find(registerAddress);
        if(iter == registerSizeMap.end() || size > iter->second){
            _log->error("Register is smaller than the requested size.");
            return -1;
        }
    }

    _log->trace("Reading {0}bytes from 0x{1}.", size, registerAddress);
    return _hal->read(LPS25H_I2C_ADDR, registerAddress, size, &data);
}

int LPS25H::writeRegister(int registerAddress, int size, byte_t &data) {
    // some size checks
    if(size < 0){
        _log->error("Cant read registers with negative size!");
        return -1;
    }

    if(size > 1){
        auto iter = registerSizeMap.find(registerAddress);
        if(iter == registerSizeMap.end() || size > iter->second){
            _log->error("Register is smaller than the requested size.");
            return -1;
        }
    }

    _log->trace("Writing {0}bytes to 0x{1}.", size, registerAddress);
    return _hal->write(LPS25H_I2C_ADDR, registerAddress, size, &data);
}

std::vector<bool> LPS25H::getResultFlags() {
    byte_t data;
    std::vector<bool> flags;

    _hal->read(LPS25H_I2C_ADDR, LPS25H_STATUS_REG, 1, &data);

    flags.push_back(data & 0x01); // temp
    flags.push_back(data & 0x02); // press

    return flags;
}

std::vector<int> LPS25H::getSettingRegisters() {
    std::vector<int> registers;

    registers.push_back(LPS25H_REF_P_XL);

    return registers;
}

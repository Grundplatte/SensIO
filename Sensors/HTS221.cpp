#include "HTS221.h"

/**
    SensIO
    HTS221.cpp

    Implementation of the sensor layer for the HTS221 sensor.

    @todo Address translation; wip
    @author Markus Feldbacher
*/

HTS221::HTS221(std::shared_ptr<HAL> hal) {
    std::shared_ptr<spdlog::logger> log = spd::get("HTS221");
    _log = log ? log : spd::stdout_color_mt("HTS221");
    _hal = hal;

    // init register size map with registers >1byte
    registerSizeMap.insert( std::pair<int, int>(HTS221_TMP_OUT_L, 2));
    registerSizeMap.insert( std::pair<int, int>(HTS221_HUM_OUT_L, 2));
}

bool HTS221::isEnabled() {
    byte_t data;
    _hal->read(HTS221_I2C_ADDR, HTS221_CTRL1, 1, &data);

    return (bool) (data & 0x87);
}

int HTS221::enable() {
    byte_t  data;

    _log->trace("Enabling Sensor.");
    _hal->read(HTS221_I2C_ADDR, HTS221_CTRL1, 1, &data);

    data |= 0x87;

    _hal->write(HTS221_I2C_ADDR, HTS221_CTRL1, 1, &data);
    return 0;
}

int HTS221::disable() {
    byte_t  data;

    _log->trace("Disabling Sensor.");
    _hal->read(HTS221_I2C_ADDR, HTS221_CTRL1, 1, &data);

    data &= ~0x80;

    _hal->write(HTS221_I2C_ADDR, HTS221_CTRL1, 1, &data);
    return 0;
}

std::vector<int> HTS221::getUnusedRegisters() {
    // not supported
    return std::vector<int>();
}

std::vector<int> HTS221::getSettingRegisters() {
    std::vector<int> registers;

    registers.push_back(HTS221_CALIB); // TODO: test influence on normal users

    return registers;
}

std::vector<bool> HTS221::getResultFlags() {
    byte_t data;
    std::vector<bool> flags;

    _hal->read(HTS221_I2C_ADDR, HTS221_STATUS, 1, &data);

    _log->warn("Status: 0x{0:2x}", data);

    flags.push_back(data & 0x01); // temp
    flags.push_back(data & 0x02); // hum

    return flags;
}

std::vector<int> HTS221::getResultRegisters() {
    std::vector<int> resultRegisters;

    // TODO: check if reading the first register is sufficient
    resultRegisters.push_back(HTS221_TMP_OUT_L); // datasheet specifies high registers must be read to reset flag
    resultRegisters.push_back(HTS221_HUM_OUT_L);

    return resultRegisters;
}

int HTS221::getSensorCount() {
    return 2;
}

int HTS221::getCycleTime() {
    return 80;
}

int HTS221::readRegister(int registerAddress, int size, byte_t &data) {
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

        registerAddress += 0x80; // increment
    }

    _log->trace("Reading {0} bytes from 0x{1:2x}", size, registerAddress);
    return _hal->read(HTS221_I2C_ADDR, registerAddress, size, &data);
}

int HTS221::writeRegister(int registerAddress, int size, byte_t &data) {
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

        registerAddress += 0x80;
    }

    _log->trace("Writing {0} bytes to 0x{1:2x}.", size, registerAddress);
    return _hal->write(HTS221_I2C_ADDR, registerAddress, size, &data);
}

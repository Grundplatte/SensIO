/**
    SensIO
    HTS221.cpp

    Implementation of the sensor layer for the HTS221 sensor.

    @todo Address translation; wip
    @author Markus Feldbacher
*/

#include "HTS221.h"

bool HTS221::isEnabled() {
    return false;
}

int HTS221::enable() {
    return 0;
}

int HTS221::disable() {
    return 0;
}

std::vector<int> HTS221::getUnusedRegisters() {
    return std::vector<int>();
}

std::vector<int> HTS221::getSettingRegisters() {
    return std::vector<int>();
}

std::vector<bool> HTS221::getResultFlags() {
    return std::vector<bool>();
}

std::vector<int> HTS221::getResultRegisters() {
    return std::vector<int>();
}

int HTS221::getSensorCount() {
    return 0;
}

int HTS221::getCycleTime() {
    return 0;
}

int HTS221::readRegister(int registerAddress, int size, byte_t &data) {
    return 0;
}

int HTS221::writeRegister(int registerAddress, int size, byte_t &data) {
    return 0;
}

/**
    SensIO
    UnusedRegisters.cpp

    Implementation of the "UnusedRegister" - Attack. This attack uses an unused sensor register (e.g. threshold, ...)
    to transmit data.

    @author Markus Feldbacher
*/

#include "UnusedRegisters.h"
#include "../Sensors/SensorBase.h"
#include "../TestBed.h"
#include "AttackHelper.h"

UnusedRegisters::UnusedRegisters(std::shared_ptr<EDC> edc, std::shared_ptr<SensorBase> sensor) {
    std::shared_ptr<spdlog::logger> log = spd::get("Unused Registers");
    _log = log ? log : spd::stdout_color_mt("Unused Registers");

    _sens = sensor;
    _edc = edc;

    // setup
    auto reg = _sens->getUnusedRegisters();
    if(reg.size() == 0){
        _log->error("Sensor has no unused registers.");
        exit(EXIT_FAILURE);
        //TODO: support waiting
    }
    else{
        // only using one register. we could speed up the transmission by using write increment with multiple registers
        _unused_reg_addr = reg.at(0);
    }

    // sender flag
    //if(TestBed::TYPE) _flag = 0x80;

    _sens->readRegister(_unused_reg_addr, 1, _last_byte);
    if(_last_byte != 0xFF){
        unsigned char data = 0xFF;
        _sens->writeRegister(_unused_reg_addr, 1, data);
        _last_byte = 0xFF;
    }
    else{
        _transmission = true;
    }
}

int UnusedRegisters::send(Packet packet) {
    _log->debug("Sending {}bit packet.", packet.size());

    int bitsize = packet.size();
    int bytesize = (bitsize-1)/7 + 1;

    std::vector<bit_t> tmp;
    packet.toBits(tmp);

    std::vector<byte_t> tmp2;
    byte_t  tmp3 = 0x00;

    // split into 7bit chunks

    int bytes = 1 + (tmp.size() - 1) / 7;

    for (int i = 0; i < bytes * 7; i++) {

        if (i < tmp.size() && tmp[i]) {
            tmp3 |= (1 << (i % 7));
        }

        if((i+1)%7==0){
            tmp2.push_back(tmp3);
            tmp3 = 0x00;
        }
    }

    for (int i = 0; i < bytesize; i++) {
        ////// WRITE
        // sender flag
        if(TestBed::TYPE){
            tmp2[i] |= 0x80;
        }

        _log->trace("sendByte: {0:x} (_last_byte {1:x})", tmp2[i], _last_byte);
        _sens->writeRegister(_unused_reg_addr, 1, tmp2[i]);
        _transmission = true;
        _last_byte = tmp2[i];


        ///// READ ACK
        byte_t data;

        _sens->readRegister(_unused_reg_addr, 1, data);

        if(!_transmission){
            // init state, check every second for request
            while (memcmp(&data, &_last_byte, 1) == 0){
                AttackHelper::waitS(1);
                _sens->readRegister(_unused_reg_addr, 1, data);
            }

            _transmission = true;
        }
        else {
            // normal receive
            while(memcmp(&data, &_last_byte, 1) == 0){
                // nothing changed
                _sens->readRegister(_unused_reg_addr, 1, data);
                //TODO: add timeout
            }

            _log->trace("readByte: {0:x}  (last_byte: {1:x})", data, _last_byte);
        }
    }

    return 0;
}

int UnusedRegisters::receive(Packet &packet, int scale) {
    // packet size for normal packets
    int packet_bitsize =
            1 + P_DATA_BITS[scale] + P_SQN_BITS + _edc->calcOutputSize(P_DATA_BITS[scale] + P_SQN_BITS + 1);

    int packet_bytesize = (packet_bitsize-1) / 7 + 1;
    _log->debug("Expecting {}byte packet.", packet_bytesize);

    std::vector<byte_t> tmp;
    std::vector<bit_t> tmp2;
    for (int i = 0; i < packet_bytesize; i++) {

        // check if new data is available. if there is new data, save it and write the inverted value to the sensor
        byte_t data;

        _sens->readRegister(_unused_reg_addr, 1, data);

        // normal receive
        while(memcmp(&data, &_last_byte, 1) == 0){
            // nothing changed
            _sens->readRegister(_unused_reg_addr, 1, data);
            //TODO: add timeout
        }

        _log->trace("readByte: {0:x}  (last_byte: {1:x})", data, _last_byte);


        //write ack
        byte_t temp = ~data & 0x7F;

        _log->trace("sendByte: {0:x} (_last_byte {1:x})", temp, _last_byte);
        _sens->writeRegister(_unused_reg_addr, 1, temp);
        _transmission = true;
        _last_byte = temp;

        tmp.push_back((unsigned char)data);

        // command packets
        if(i == 0 && data & 0x01){
            packet_bytesize = 2;
        }
    }

    // 7bit -> bit-array
    for (int i = 0; i < tmp.size(); i++) {
        for (int l = 0; l < 7; l++) {
            tmp2.push_back((unsigned char)((tmp[i] & (1 << l)) >> l));
        }
    }

    for(int i=0; i<tmp.size(); i++){
        _log->trace("byte: {0:x}", tmp[i]);
    }

    for(int i=0; i<tmp2.size(); i++){
        _log->trace("bit: {0:x}", tmp2[i]);
    }


    packet.fromBits(tmp2, scale);

    return 0;
}

int UnusedRegisters::request(byte_t req) {

    _sens->writeRegister(_unused_reg_addr, 1, req);
    _last_byte = req;

    //TODO: error handling

    return 0;
}

int UnusedRegisters::waitForRequest() {
    byte_t byte;

    _sens->readRegister(_unused_reg_addr, 1, byte);
    while(memcmp(&byte, &_last_byte, 1) == 0) {
        if(_transmission) AttackHelper::waitS(1); // wait 1sec

        _sens->readRegister(_unused_reg_addr, 1, byte);
    }

    _transmission = true;
    _log->debug("Received sqnHad: 0x{0:2x}", byte);

    return byte; //FIXME: conversion
}

int UnusedRegisters::wait(int cycles) {
    // not needed
    return 0;
}

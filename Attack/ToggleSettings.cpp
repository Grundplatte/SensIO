//
// Created by Markus Feldbacher on 12.04.18.
//

#include "ToggleSettings.h"
#include "../TestBed.h"
#include "../PacketSystem/ECC/Hadamard.h"

ToggleSettings::ToggleSettings(std::shared_ptr<EDC> edc, std::shared_ptr<SensorBase> sensor) {
    std::shared_ptr<spdlog::logger> log = spd::get("Toggle Settings");
    _log = log ? log : spd::stdout_color_mt("Toggle Settings");

    _sens = sensor;
    _edc = edc;

    // setup
    auto reg = _sens->getSettingRegisters();
    if(reg.size() == 0){
        _log->error("Sensor has no usable setting registers.");
        exit(EXIT_FAILURE);
        //TODO: support waiting
    }
    else{
        _setting_reg_addr = reg.at(0);
    }

    // isActive not needed
    _sens->readRegister(_setting_reg_addr, 1, _ref_value);
    _log->trace("Initial state: {0:x}", _ref_value);
    if(_ref_value != (_ref_value & 0xFC)){
        _ref_value &= 0xFC;
        _sens->writeRegister(_setting_reg_addr, 1, _ref_value);
    }

    _last_bit = 0;
}

int ToggleSettings::send(Packet packet) {
    unsigned char data;
    data = _ref_value;

    for(int i=0; i<packet.getSize(); i++) {
        if (packet[i] != 0) {
            data |= 0x01;
        }

        // senderflag
        if (TestBed::TYPE) data |= 0x02;

        _log->trace("Sent bit {0:x} ({1:x})", data & 0x01, data & 0x02);
        _sens->writeRegister(_setting_reg_addr, 1, data);
        _last_bit = !_last_bit;

        // get ack
        bool new_bit;
        do {
            _sens->readRegister(_setting_reg_addr, 1, data);

            new_bit = (bool) (data & 0x02);

        } while (new_bit == _last_bit);

        // new data available
        _last_bit = new_bit;
        _log->trace("Received ACK ({0:x})", data & 0x02);
    }
    return 0;
}

int ToggleSettings::receive(Packet &packet, int scale) {

    // packet size for normal packets
    size_t packet_bitsize =
            1 + P_DATA_BITS[scale] + P_SQN_BITS + _edc->calcOutputSize(P_DATA_BITS[scale] + P_SQN_BITS + 1);
    _log->debug("Expecting {}bit packet.", packet_bitsize);

    // clean up
    std::vector<bit_t> tmp;
    for (int i = 0; i < packet_bitsize; i++) {
        //bit = _sens->readBit(i, long_timeout ? 3 : 2);

        unsigned char data;
        unsigned char ack;
        bool new_bit;

        do {
            _sens->readRegister(_setting_reg_addr, 1, data);

            new_bit = (bool) (data & 0x02);

        } while(new_bit == _last_bit);

        // new data available
        _last_bit = new_bit;
        _log->trace("Received bit {0:x} ({1:x})", data & 0x01, data & 0x02);

        // write ack
        ack = _ref_value;
        if(!_last_bit){
            ack |= 0x02;
        }
        _sens->writeRegister(_setting_reg_addr, 1, ack);
        _last_bit = !_last_bit;
        _log->trace("Sent ACK ({0:x})", ack & 0x02);

        //return data[0] & 0x01;

        if (data) {
            if (tmp.empty()) {
                // packet size for command packets
                packet_bitsize = 1 + P_CMD_BITS + P_SQN_BITS + _edc->calcOutputSize(P_CMD_BITS + P_SQN_BITS + 1);
            }

            tmp.push_back(1);
        } else {
            tmp.push_back(0);
        }
    }

    packet.fromBits(tmp, scale);

    return 0;
}

int ToggleSettings::request(byte_t req) {
    Hadamard ecc;
    int had_bitsize = ecc.getEncodedSize(P_SQN_BITS);

    if (had_bitsize < 0) {
        return -1;
    }

    for (int i = 0; i < had_bitsize; i++) {
        bit_t bit = (bit_t) (req & (1 << (i % 8)));
        //_sens->sendBit((bit_t) (sqn_had & (1 << (i % 8))));
        unsigned char data;
        data = _ref_value;

        if(bit != 0){
            data |= 0x01;
        }

        if(!_last_bit){
            data |= 0x02;
        }

        _log->trace("Sent bit {0:x} ({1:x})", data & 0x01, data & 0x02);
        _sens->writeRegister(_setting_reg_addr, 1, data);
        _last_bit = !_last_bit;

        // get ack
        bool new_bit;
        do {
            _sens->readRegister(_setting_reg_addr, 1, data);

            new_bit = (bool) (data & 0x02);

        } while(new_bit == _last_bit);

        // new data available
        _last_bit = new_bit;
        _log->trace("Received ACK ({0:x})", data & 0x02);
    }
    return 0;
}

int ToggleSettings::waitForRequest() {
    byte_t byte = 0x00;

    Hadamard ecc;
    int had_bitsize = ecc.getEncodedSize(P_SQN_BITS);

    for (int i = 0; i < had_bitsize; i++) {
        //bit = _sens->readBit(i, long_timeout ? 3 : 2);

        unsigned char data;
        unsigned char ack;
        bool new_bit;

        do {
            _sens->readRegister(_setting_reg_addr, 1, data);

            new_bit = (bool) (data & 0x02);

        } while(new_bit == _last_bit);

        // new data available
        _last_bit = new_bit;
        _log->trace("Received bit {0:x} ({1:x})", data & 0x01, data & 0x02);

        // write ack
        ack = _ref_value;
        if(!_last_bit){
            ack |= 0x02;
        }
        _sens->writeRegister(_setting_reg_addr, 1, ack);
        _last_bit = !_last_bit;
        _log->trace("Sent ACK ({0:x})", ack & 0x02);

        if (data) {
            byte |= (1 << i);
        }
    }

    _log->debug("Received sqnHad: 0x{0:2x}", byte);
    return byte;
}

int ToggleSettings::wait(int cycles) {
    return 0;
}

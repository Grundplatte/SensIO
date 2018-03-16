//
// Created by Markus Feldbacher on 20.12.17.
//

#include <sstream>
#include "Packet.h"
#include "EDC/Berger.h"


Packet::Packet(std::shared_ptr<EDC> edc) : _edc(edc) {
    std::shared_ptr<spdlog::logger> log = spd::get("Packet");
    _log = log ? log : spd::stdout_color_mt("Packet");
}

Packet::Packet(std::vector<bit_t> data, int sqn, std::shared_ptr<EDC> edc) : _data_bits(std::move(data)), _sqn(sqn), _type(TYPE_DATA), _edc(edc) {
    // construct data packet
    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("Packet");
    _log = log ? log : spd::stdout_color_mt("Packet");

    updateSQN();
    updateEDC();
}

Packet::Packet(int cmd, int sqn, std::shared_ptr<EDC> edc) : _sqn(sqn), _type(TYPE_CMD), _edc(edc) {
    // construct cmd packet (always 2bit data)
    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("Packet");
    _log = log ? log : spd::stdout_color_mt("Packet");

    if (cmd < 0 || cmd > 3)
        _log->error("Unknown Command!");

    _data_bits.clear();
    _data_bits.push_back((unsigned char) (cmd & 0x01));
    _data_bits.push_back((unsigned char) ((cmd & 0x02) >> 1));

    updateSQN();
    updateEDC();
}

void Packet::toBits(std::vector<bit_t> &output) {
    output.clear();
    output.push_back(_type);
    output.insert(output.end(), _data_bits.begin(), _data_bits.end());
    output.insert(output.end(), _sqn_bits.begin(), _sqn_bits.end());
    output.insert(output.end(), _edc_bits.begin(), _edc_bits.end());
}

void Packet::setSqn(int new_sqn) {
    _sqn = new_sqn;
    updateSQN();
    updateEDC();
}

int Packet::size() {
    return _data_bits.size() + _sqn_bits.size() + _edc_bits.size() + 1;
}

std::vector<bit_t> Packet::getData() {
    return _data_bits;
}

void Packet::fromBits(std::vector<bit_t> input, int scale) {
    _type = input[0];

    _data_bits.clear();
    _sqn_bits.clear();
    _edc_bits.clear();

    if (_type == TYPE_DATA) {
        _data_bits.insert(_data_bits.end(), input.begin() + 1, input.begin() + P_DATA_BITS[scale] + 1);
        _sqn_bits.insert(_sqn_bits.end(), input.begin() + P_DATA_BITS[scale] + 1,
                        input.begin() + P_DATA_BITS[scale] + P_SQN_BITS + 1);

        int edc_size = _edc->calcOutputSize(_data_bits.size() + _sqn_bits.size() + 1);
        _edc_bits.insert(_edc_bits.end(), input.begin() + P_DATA_BITS[scale] + P_SQN_BITS + 1,
                         input.begin() + P_DATA_BITS[scale] + P_SQN_BITS + 1 + edc_size);
    } else if (_type == TYPE_CMD) {
        _data_bits.insert(_data_bits.end(), input.begin() + 1, input.begin() + P_CMD_BITS + 1);
        _sqn_bits.insert(_sqn_bits.end(), input.begin() + P_CMD_BITS + 1, input.begin() + P_CMD_BITS + P_SQN_BITS + 1);

        Berger edc;
        int edc_size = edc.calcOutputSize(_data_bits.size() + _sqn_bits.size() + 1);
        _edc_bits.insert(_edc_bits.end(), input.begin() + P_CMD_BITS + P_SQN_BITS + 1,
                         input.begin() + P_CMD_BITS + P_SQN_BITS + 1 + edc_size);
    }

    _sqn = -1;
}

void Packet::updateEDC() {
    std::vector<bit_t> tmp(_data_bits);
    tmp.insert(tmp.end(), _sqn_bits.begin(), _sqn_bits.end());
    tmp.insert(tmp.begin(), _type);

    _edc_bits.clear();

    // FIX: allways use edc for command packets
    if(isCommand()){
        Berger edc;
        edc.generate(tmp, _edc_bits);
    }
    else{
        _edc->generate(tmp, _edc_bits);
    }
}

void Packet::updateSQN() {
    int mod_sqn = _sqn % (MAX_SQN + 1);
    _sqn_bits.clear();
    // add sqn bits
    for (int local_i = 0; local_i < P_SQN_BITS; local_i++) {
        _sqn_bits.push_back((bit_t) (mod_sqn & (1 << local_i)) >> local_i);
    }
}

bit_t const &Packet::operator[](int index) {
    if (index == 0)
        return _type;
    else if (index <= _data_bits.size())
        return _data_bits[index - 1];
    else if (index <= _data_bits.size() + _sqn_bits.size())
        return _sqn_bits[index - _data_bits.size() - 1];
    else if (index <= _data_bits.size() + _sqn_bits.size() + _edc_bits.size())
        return _edc_bits[index - _data_bits.size() - _sqn_bits.size() - 1];

    // bad solution
    return 0;
}

int Packet::isValid() {
    std::vector<bit_t> tmp(_data_bits);
    tmp.insert(tmp.end(), _sqn_bits.begin(), _sqn_bits.end());
    tmp.insert(tmp.begin(), _type);

    int check;
    if(isCommand()){
        Berger edc;
        check = edc.check(tmp, _edc_bits);
    }
    else {
        check = _edc->check(tmp, _edc_bits);
    }

    return (check == 1 ? 1 : 0);
}

int Packet::hasSqn(int sqn) {
    if (sqn == _sqn)
        return 1;

    if (_sqn == -1) {
        int mod_sqn = sqn % (MAX_SQN + 1);
        std::vector<bit_t> sqn_bits;
        // add sqn bits
        for (int i = 0; i < P_SQN_BITS; i++) {
            sqn_bits.push_back((bit_t) (mod_sqn & (1 << i)) >> i);
        }

        for (int i = 0; i < sqn_bits.size(); i++) {
            if (sqn_bits[i] != _sqn_bits[i])
                return 0;
        }

        _sqn = sqn;
        return 1;
    }

    return 0;
}

void Packet::printContents() {
    if (!_log->should_log(spd::level::debug))
        return;

    std::stringstream temp;

    _log->debug("===== Packet =====");
    _log->debug("= SQN: {0:2}", _sqn);
    _log->debug("= TYPE: {0}", _type);
    _log->debug("===================");

    temp.str("");
    for (unsigned char data_bit : _data_bits) {
        temp << (data_bit ? "1" : "0");
    }
    if (_type == TYPE_CMD) {
        int cmd = 0x00 | _data_bits[0] | _data_bits[1];
        switch (cmd) {
            case CMD_UP:
                temp << " (CMD_UP)";
                break;
            case CMD_DOWN:
                temp << " (CMD_DOWN)";
                break;
            case CMD_STOP:
                temp << " (CMD_STOP)";
                break;
            case CMD_REV:
                temp << " (CMD_REV)";
                break;
            default:
                temp << " (unknown)";
                break;
        }
    }
    _log->debug("= DATA: {0:s}", temp.str());

    temp.str("");
    for (unsigned char sqn_bit : _sqn_bits) {
        temp << (sqn_bit ? "1" : "0");
    }
    _log->debug("= SQN: {0:s} (max sqn: {1})", temp.str(), MAX_SQN);

    temp.str("");
    for (unsigned char edc_bit : _edc_bits) {
        temp << (edc_bit ? "1" : "0");
    }
    _log->debug("= EDC : {0:s}", temp.str());

    _log->debug("====================");
    _log->debug("");
}

int Packet::isCommand() {
    return _type == TYPE_CMD;
}

int Packet::getCommand() {
    if (!isCommand())
        return -1;

    int cmd = 0;
    cmd |= _data_bits[0];
    cmd |= _data_bits[1] << 1;

    return cmd;
}

int Packet::getSize() {
    return _data_bits.size() + _edc_bits.size() + _sqn_bits.size();
}

/**
    SensIO
    ToggleSettings.cpp

    Implementation of the "ToggleSettings" - Attack. Transmits data by slightly altering suitable sensor configurations
    (e.g. Thresholds, Reference values, ...)

    @author Markus Feldbacher
*/

#include "ToggleSettings.h"
#include "../TestBed.h"
#include "../PacketSystem/ECC/Hadamard.h"
#include "AttackHelper.h"

ToggleSettings::ToggleSettings(std::shared_ptr<EDC> edc, std::shared_ptr<SensorBase> sensor) {
    std::shared_ptr<spdlog::logger> log = spd::get("Toggle Settings");
    _log = log ? log : spd::stdout_color_mt("Toggle Settings");

    _sens = sensor;
    _edc = edc;

    std::vector<int> reg = _sens->getSettingRegisters();
    int retrys = 0;

    while(reg.size() == 0){
        if(++retrys > MAX_RETRYS){
            _log->error("Sensor has no usable setting registers.");
            exit(EXIT_FAILURE);
        }

        AttackHelper::waitS(10); // retry after 10 seconds

        reg = _sens->getSettingRegisters();
    }

    _setting_reg_addr = reg.at(0);

    byte_t ref_value;
    _sens->readRegister(_setting_reg_addr, 1, ref_value);
    _log->trace("Initial state: {0:x}", ref_value);
    if(ref_value != (ref_value & RESET_MASK)){
        ref_value &= 0xFC;
        _sens->writeRegister(_setting_reg_addr, 1, ref_value);
    }
}

int ToggleSettings::send(Packet packet) {
    byte_t data;
    _sens->readRegister(_setting_reg_addr, 1, data);

    data &= RESET_MASK;

    for(int i=0; i<packet.size(); i++) {
        if (packet[i] != 0) {
            data |= DATA_MASK;
        }

        write(data);
        waitForAck();

        data &= RESET_MASK;
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
        byte_t data;

        read(data);
        ack(data);

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

    byte_t data;
    _sens->readRegister(_setting_reg_addr, 1, data);

    for (int i = 0; i < had_bitsize; i++) {
        bit_t bit = (bit_t) (req & (1 << (i % 8)));

        if(bit != 0){
            data |= 0x01;
        }

        write(data);
        waitForAck();

        data &= RESET_MASK;
    }
    return 0;
}

int ToggleSettings::waitForRequest() {
    byte_t byte = 0x00;

    Hadamard ecc;
    int had_bitsize = ecc.getEncodedSize(P_SQN_BITS);

    for (int i = 0; i < had_bitsize; i++) {
        byte_t data;

        read(data);
        ack(data);

        if (data & DATA_MASK) {
            byte |= (1 << i);
        }
    }

    _log->debug("Received sqnHad: 0x{0:2x}", byte);
    return byte;
}

void ToggleSettings::wait(int cycles) {
    long ms = _sens->getCycleTime();
    AttackHelper::waitMs(ms * cycles);
}


/////// PRIVATE ///////

int ToggleSettings::write(byte_t &data) {
    data &= ~FLAG_MASK;

    _last_bit = !_last_bit;

    data = _last_bit ? data | FLAG_MASK : data;

    _sens->writeRegister(_setting_reg_addr, 1, data);
    _log->trace("Sent bit {0:x} ({1:x})", data & DATA_MASK, data & FLAG_MASK);
    return 0;
}



int ToggleSettings::read(byte_t &data) {
    do {
        _sens->readRegister(_setting_reg_addr, 1, data);
        // TODO: timeout
    } while((bool)(data & FLAG_MASK) == _last_bit);

    _log->trace("Received bit {0:x} ({1:x})", data & DATA_MASK, data & FLAG_MASK);

    _last_bit = !_last_bit;
    return 0;
}

int ToggleSettings::waitForAck() {
    byte_t data;
    read(data);
    _log->trace("Received ACK ({0:x})", data & FLAG_MASK);

    return 0;
}

int ToggleSettings::ack(byte_t &data) {
    write(data);
    _log->trace("Sent ACK ({0:x})", data & FLAG_MASK);

    return 0;
}

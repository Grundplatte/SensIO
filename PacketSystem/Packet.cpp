//
// Created by Markus Feldbacher on 20.12.17.
//

#include <sstream>
#include "Packet.h"
#include "EDC/Berger.h"


Packet::Packet() {
    std::shared_ptr<spdlog::logger> log = spd::get("Packet");
    m_log = log ? log : spd::stdout_color_mt("Packet");
}

Packet::Packet(std::vector<bit_t> data, int sqn) : data_bits(std::move(data)), sqn(sqn), type(TYPE_DATA) {
    // construct data packet
    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("Packet");
    m_log = log ? log : spd::stdout_color_mt("Packet");

    // if packet is smaller than packetsize, fill with zeros
    while (data_bits.size() < P_DATA_BITS) {
        data_bits.push_back(0);
    }

    updateSQN();
    updateEDC();
}

Packet::Packet(int cmd, int sqn) : sqn(sqn), type(TYPE_CMD) {
    // construct cmd packet (always 2bit data)
    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("Packet");
    m_log = log ? log : spd::stdout_color_mt("Packet");

    if (cmd < 0 || cmd > 3)
        m_log->error("Unknown Command!");

    data_bits.clear();
    data_bits.push_back(cmd & 0x01);
    data_bits.push_back((cmd & 0x02) >> 1);

    updateSQN();
    updateEDC();
}

void Packet::toBits(std::vector<bit_t> &output) {
    output.clear();
    output.push_back(type);
    output.insert(output.end(), data_bits.begin(), data_bits.end());
    output.insert(output.end(), sqn_bits.begin(), sqn_bits.end());
    output.insert(output.end(), edc_bits.begin(), edc_bits.end());
}

void Packet::setSqn(int new_sqn) {
    sqn = new_sqn;
    updateSQN();
    updateEDC();
}

int Packet::size() {
    return data_bits.size() + sqn_bits.size() + edc_bits.size() + 1;
}

std::vector<bit_t> Packet::getData() {
    return data_bits;
}

void Packet::fromBits(std::vector<bit_t> input) {
    type = input[0];

    data_bits.clear();
    sqn_bits.clear();
    edc_bits.clear();

    if (type == TYPE_DATA) {
        data_bits.insert(data_bits.end(), input.begin() + 1, input.begin() + P_DATA_BITS + 1);
        sqn_bits.insert(sqn_bits.end(), input.begin() + P_DATA_BITS + 1, input.begin() + P_DATA_BITS + P_SQN_BITS + 1);
        edc_bits.insert(edc_bits.end(), input.begin() + P_DATA_BITS + P_SQN_BITS + 1, input.end());
    } else if (type == TYPE_CMD) {
        data_bits.insert(data_bits.end(), input.begin() + 1, input.begin() + P_CMD_BITS + 1);
        sqn_bits.insert(sqn_bits.end(), input.begin() + P_CMD_BITS + 1, input.begin() + P_CMD_BITS + P_SQN_BITS + 1);
        edc_bits.insert(edc_bits.end(), input.begin() + P_CMD_BITS + P_SQN_BITS + 1, input.end());
    }

    sqn = -1;
}

void Packet::updateEDC() {
    EDC *edc = new Berger();
    std::vector<bit_t> tmp(data_bits);
    tmp.insert(tmp.end(), sqn_bits.begin(), sqn_bits.end());
    tmp.insert(tmp.begin(), type);

    edc_bits.clear();
    edc->generate(tmp, edc_bits);
}

void Packet::updateSQN() {
    int mod_sqn = sqn % (MAX_SQN + 1);
    sqn_bits.clear();
    // add sqn bits
    for (int local_i = 0; local_i < P_SQN_BITS; local_i++) {
        sqn_bits.push_back((bit_t) (mod_sqn & (1 << local_i)) >> local_i);
    }
}

bit_t const &Packet::operator[](int index) {
    if (index == 0)
        return type;
    else if (index <= data_bits.size())
        return data_bits[index - 1];
    else if (index <= data_bits.size() + sqn_bits.size())
        return sqn_bits[index - data_bits.size() - 1];
    else if (index <= data_bits.size() + sqn_bits.size() + edc_bits.size())
        return edc_bits[index - data_bits.size() - sqn_bits.size() - 1];

    // bad solution
    return 0;
}

int Packet::isValid() {
    EDC *edc = new Berger();
    std::vector<bit_t> tmp(data_bits);
    tmp.insert(tmp.end(), sqn_bits.begin(), sqn_bits.end());
    tmp.insert(tmp.begin(), type);

    int check = edc->check(tmp, edc_bits);

    return (check == 1 ? 1 : 0);
}

int Packet::hasSqn(int sqn) {
    if (sqn == this->sqn)
        return 1;

    if (this->sqn == -1) {
        int modSqn = sqn % (MAX_SQN + 1);
        std::vector<bit_t> sqn_bits;
        // add sqn bits
        for (int i = 0; i < P_SQN_BITS; i++) {
            sqn_bits.push_back((bit_t) (modSqn & (1 << i) >> i));
        }

        for (int i = 0; i < sqn_bits.size(); i++) {
            if (sqn_bits[i] && !this->sqn_bits[i])
                return 0;

            if (!sqn_bits[i] && this->sqn_bits[i])
                return 0;
        }

        this->sqn = sqn;
        return 1;
    }

    return 0;
}

void Packet::printContents() {
    if (!m_log->should_log(spd::level::debug))
        return;

    std::stringstream temp;

    m_log->debug("===== Packet =====");
    m_log->debug("= SQN: {0:2}", sqn);
    m_log->debug("= TYPE: {0}", type);
    m_log->debug("===================");

    temp.str("");
    for (int i = 0; i < data_bits.size(); i++) {
        temp << (data_bits[i] ? "1" : "0");
    }
    m_log->debug("= DATA: {0:s}", temp.str());

    temp.str("");
    for (int i = 0; i < sqn_bits.size(); i++) {
        temp << (sqn_bits[i] ? "1" : "0");
    }
    m_log->debug("= SQN: {0:s} (max sqn: {1})", temp.str(), MAX_SQN);

    temp.str("");
    for (int i = 0; i < edc_bits.size(); i++) {
        temp << (edc_bits[i] ? "1" : "0");
    }
    m_log->debug("= EDC : {0:s}", temp.str());

    m_log->debug("====================");
    m_log->debug("");
}

int Packet::isCommand() {
    return type == TYPE_CMD;
}

int Packet::getCommand() {
    if (!isCommand())
        return -1;

    int cmd = 0;
    cmd |= data_bits[0];
    cmd |= data_bits[1] << 1;

    return cmd;
}

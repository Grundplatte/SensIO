//
// Created by Markus Feldbacher on 20.12.17.
//

#ifndef SIDECHANNEL_PACKET_H
#define SIDECHANNEL_PACKET_H


#include <vector>
#include "../Defines.h"
#include "../spdlog/logger.h"

namespace spd = spdlog;

class Packet {
public:
    // not used for now
    static const int TYPE_DATA = 0;
    static const int TYPE_CMD = 1;

    static const int CMD_UP = 0;
    static const int CMD_DOWN = 1;
    static const int CMD_STOP = 2;
    static const int CMD_RES = 3;

    Packet(std::vector<bit_t> data, int sqn);

    Packet(int cmd, int sqn);

    Packet();

    int size();

    int isValid();

    int isCommand();

    int hasSqn(int sqn);

    void toBits(std::vector<bit_t> &output);

    void fromBits(std::vector<bit_t> input, int scale);

    void setSqn(int new_sqn);

    std::vector<bit_t> getData();

    int getCommand();

    void printContents();

    bit_t const &operator[](int index);

private:
    std::shared_ptr<spd::logger> m_log;

    std::vector<bit_t> data_bits;
    std::vector<bit_t> sqn_bits;
    std::vector<bit_t> edc_bits;

    int sqn;
    bit_t type;

    void updateEDC();

    void updateSQN();
};


#endif //SIDECHANNEL_PACKET_H

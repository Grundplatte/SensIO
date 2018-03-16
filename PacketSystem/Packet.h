//
// Created by Markus Feldbacher on 20.12.17.
//

#ifndef SIDECHANNEL_PACKET_H
#define SIDECHANNEL_PACKET_H


#include <vector>
#include "../Defines.h"
#include "../spdlog/logger.h"
#include "EDC/EDC.h"

namespace spd = spdlog;

class Packet {
public:
    // not used for now
    static const int TYPE_DATA = 0;
    static const int TYPE_CMD = 1;

    static const int CMD_UP = 0;
    static const int CMD_DOWN = 1;
    static const int CMD_STOP = 2;
    static const int CMD_REV = 3;

    Packet(std::vector<bit_t> data, int sqn, std::shared_ptr<EDC> _edc);

    Packet(int cmd, int sqn, std::shared_ptr<EDC> _edc);

    Packet(std::shared_ptr<EDC> _edc);

    int size();

    int isValid();

    int isCommand();

    int hasSqn(int sqn);

    void toBits(std::vector<bit_t> &output);

    void fromBits(std::vector<bit_t> input, int scale);

    void setSqn(int new_sqn);

    std::vector<bit_t> getData();

    int getSize();

    int getCommand();

    void printContents();

    bit_t const &operator[](int index);

private:
    std::shared_ptr<spd::logger> _log;

    std::vector<bit_t> _data_bits;
    std::vector<bit_t> _sqn_bits;
    std::vector<bit_t> _edc_bits;

    std::shared_ptr<EDC> _edc;

    int _sqn;
    bit_t _type;

    void updateEDC();

    void updateSQN();
};


#endif //SIDECHANNEL_PACKET_H

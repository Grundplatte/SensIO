//
// Created by Markus Feldbacher on 04.01.18.
//

#ifndef SIDECHANNEL_PACKETFACTORY_H
#define SIDECHANNEL_PACKETFACTORY_H


#include <vector>
#include "../Defines.h"
#include "../spdlog/logger.h"
#include "Packet.h"

namespace spd=spdlog;

class PacketFactory {
public:
    PacketFactory();

    explicit PacketFactory(unsigned char *data_in, size_t length);

    void resetIterator();

    void appendData(unsigned char *data_add, size_t length);

    int getNextPacket(int sqn, Packet &ret);

    int getCommandPacket(int cmd, int sqn, Packet &ret);

    int previous(); // bad name
    int scaleUp();

    int scaleDown();

private:
    std::shared_ptr<spd::logger> _log;

    std::vector<bit_t> _data;
    std::vector<bit_t>::iterator _last_packet_start;
    std::vector<bit_t>::iterator _next_packet_start;

    // dynamic stuff
    int _scale = P_INIT_SCALE;
};


#endif //SIDECHANNEL_PACKETFACTORY_H

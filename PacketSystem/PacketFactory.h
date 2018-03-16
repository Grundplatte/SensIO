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
    PacketFactory(std::shared_ptr<EDC> edc);

    explicit PacketFactory(unsigned char *data_in, size_t length, std::shared_ptr<EDC> edc);

    void resetIterator();

    void appendData(unsigned char *data_add, size_t length);

    int getNextPacket(int sqn, Packet &ret);

    int getCommandPacket(int cmd, int sqn, Packet &ret);

    int previous(); // bad name

    static int scaleUp();

    static int scaleDown();

    static int getScale();

    bool isEmpty() { return _data.size(); }

private:
    std::shared_ptr<spd::logger> _log;

    std::shared_ptr<EDC> _edc;

    std::vector<bit_t> _data;
    std::vector<bit_t>::iterator _last_packet_start;
    std::vector<bit_t>::iterator _next_packet_start;

    // dynamic stuff
    static int _scale;
};


#endif //SIDECHANNEL_PACKETFACTORY_H

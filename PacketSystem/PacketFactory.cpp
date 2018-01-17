//
// Created by Markus Feldbacher on 04.01.18.
//

#include "PacketFactory.h"
#include "../spdlog/spdlog.h"

PacketFactory::PacketFactory() {
    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("PMgr");
    m_log = log ? log : spd::stdout_color_mt("PMgr");

    next_packet_start = data.begin();
    last_packet_start = data.begin();
}

PacketFactory::PacketFactory(unsigned char *data_in, size_t length) {
    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("PMgr");
    m_log = log ? log : spd::stdout_color_mt("PMgr");

    appendData(data_in, length);

    next_packet_start = data.begin();
    last_packet_start = data.begin();
}

void PacketFactory::appendData(unsigned char *data_add, size_t length) {
    // TODO convert data_in to bits
    for (int i = 0; i < length; i++) {
        for (int l = 0; l < 8; l++) {
            data.push_back(((data_add[i] & (1 << l)) >> l));
            m_log->trace("Data bit {0:3}/{2:3}: {1}", i * 8 + l, data[i * 8 + l], length * 8);
        }
    }
}

int PacketFactory::getNextPacket(int sqn, Packet &ret) {

    int next_packet_count = 0;
    std::vector<bit_t> tmp;

    if (next_packet_start == data.end())
        return -1; // no data left


    // TODO: testing scaledown if not much data left (needs better logic)
    int dist = std::distance(next_packet_start, data.end());
    m_log->debug("Data left: {}", dist);
    if (dist * 3 < P_DATA_BITS[packet_scale]) {
        scaleDown();
        ret = Packet(Packet::CMD_DOWN, sqn);
        return 0;
    }


    last_packet_start = next_packet_start;

    while (next_packet_start != data.end() && next_packet_count < P_DATA_BITS[packet_scale]) {
        tmp.push_back(*next_packet_start++);
        next_packet_count++;
    }

    while (next_packet_count < P_DATA_BITS[packet_scale]) {
        // not enough data left, filling with zeros
        tmp.push_back(0);
        next_packet_count++;
    }

    ret = Packet(tmp, sqn);
    return 0;
}

int PacketFactory::previous() {
    next_packet_start = last_packet_start;
    return 0;
}

int PacketFactory::getCommandPacket(int cmd, int sqn, Packet &ret) {
    if (cmd < 0 || cmd > 3)
        return -1;

    ret = Packet(cmd, sqn);
    return 0;
}

void PacketFactory::resetIterator() {
    next_packet_start = data.begin();
}

int PacketFactory::scaleUp() {
    if (packet_scale < sizeof(P_DATA_BITS)) {
        packet_scale++;
        return 0;
    }
    return -1;
}

int PacketFactory::scaleDown() {
    if (packet_scale > 0) {
        packet_scale--;
        return 0;
    }
    return -1;
}

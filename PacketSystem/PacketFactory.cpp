/**
    SensIO
    PacketFactory.cpp

    A Factory used to generate packets from data (bytes).

    @author Markus Feldbacher
*/

#include "PacketFactory.h"
#include "../spdlog/spdlog.h"

int PacketFactory::_scale = P_INIT_SCALE;

PacketFactory::PacketFactory(std::shared_ptr<EDC> edc): _edc(edc) {
    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("PF");
    _log = log ? log : spd::stdout_color_mt("PF");

    _next_packet_start = _data.begin();
    _last_packet_start = _data.begin();
}

PacketFactory::PacketFactory(unsigned char *data_in, size_t length, std::shared_ptr<EDC> edc) : _edc(edc) {
    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("PF");
    _log = log ? log : spd::stdout_color_mt("PF");

    appendData(data_in, length);

    _next_packet_start = _data.begin();
    _last_packet_start = _data.begin();
}

void PacketFactory::appendData(unsigned char *data_add, size_t length) {
    int offset_next = std::distance(_next_packet_start, _data.begin());
    int offset_last = std::distance(_last_packet_start, _data.begin());

    for (int i = 0; i < length; i++) {
        for (int l = 0; l < 8; l++) {
            _data.push_back((unsigned char) ((data_add[i] & (1 << l)) >> l));
            _log->trace("Data bit {0:3}/{2:3}: {1}", i * 8 + l, _data[i * 8 + l], length * 8);
        }
    }

    _next_packet_start = _data.begin() + offset_next;
    _last_packet_start = _data.begin() + offset_last;
}

int PacketFactory::getNextPacket(int sqn, Packet &ret) {

    int next_packet_count = 0;
    std::vector<bit_t> tmp;

    if (_next_packet_start == _data.end())
        return -1; // no data left


    // TODO: testing scaledown if not much data left (needs better logic)
    int dist = std::distance(_next_packet_start, _data.end());
    _log->debug("Data left: {}", dist);
    if (dist * 3 < P_DATA_BITS[_scale] && scaleDown() == 0) {
        ret = Packet(Packet::CMD_DOWN, sqn, _edc);
        return 0;
    }


    _last_packet_start = _next_packet_start;

    while (_next_packet_start != _data.end() && next_packet_count < P_DATA_BITS[_scale]) {
        tmp.push_back(*_next_packet_start++);
        next_packet_count++;
    }

    while (next_packet_count < P_DATA_BITS[_scale]) {
        // not enough data left, filling with zeros
        tmp.push_back(0);
        next_packet_count++;
    }

    ret = Packet(tmp, sqn, _edc);
    return 0;
}

int PacketFactory::previous() {
    _next_packet_start = _last_packet_start;
    return 0;
}

int PacketFactory::getCommandPacket(int cmd, int sqn, Packet &ret) {
    if (cmd < 0 || cmd > 3)
        return -1;

    ret = Packet(cmd, sqn, _edc);
    return 0;
}

void PacketFactory::resetIterator() {
    _next_packet_start = _data.begin();
}

int PacketFactory::scaleUp() {
#ifdef DYNAMIC
    if (_scale < 2) {
        _scale++;
        return 0;
    }
#endif
    return -1;
}

int PacketFactory::scaleDown() {
#ifdef DYNAMIC
    if (_scale > 0) {
        _scale--;
        return 0;
    }
#endif
    return -1;
}

int PacketFactory::getScale() {
    return _scale;
}

#include <cmath>
#include <utility>
#include <sstream>
#include <iostream>
#include "PacketManager.h"
#include "PacketHelper.h"
#include "EDC/Berger.h"
#include "../TestBed.h"

namespace spd = spdlog;

PacketManager::PacketManager(std::shared_ptr<ECC> ecc, std::shared_ptr<EDC> edc, std::shared_ptr<Sensor> sensor) : _ecc(
        ecc), _edc(edc), _sens(sensor) {
    if (!_sens->isActive())
        _sens->toggleOnOff(1);

    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("PMgr");
    _log = log ? log : spd::stdout_color_mt("PMgr");
}

/**************************
 * PUBLIC FUNCTIONS
***************************/

int PacketManager::waitForRequest(byte_t *sqn_had) {
    return _sens->supportsBytes() ? waitForRequestByte(sqn_had) : waitForRequestBits(sqn_had);
}

int PacketManager::checkForRequest(byte_t *sqn_had, bool long_timeout) {
    return _sens->supportsBytes() ? checkForRequestByte(sqn_had, long_timeout) : checkForRequestBits(sqn_had, long_timeout);
}

int PacketManager::request(int sqn) {
    return _sens->supportsBytes() ? requestBytes(sqn) : requestBits(sqn);
}

int PacketManager::validateRequest(byte_t *sqn_had) {
    size_t had_length = (size_t) _ecc->getEncodedSize(P_SQN_BITS);
    // error correction
    // return ecc.decode(sqnHad, hda_length, sqn)

    // error detection
    return _ecc->check(sqn_had, had_length);
}

// send packet
int PacketManager::send(Packet packet) {
    return _sens->supportsBytes() ? sendBytes(packet) : sendBits(packet);
}

int PacketManager::receive(Packet &packet, int sqn, int scale, int long_timeout) {
    return _sens->supportsBytes() ? receiveBytes(packet, sqn, scale, long_timeout) : receiveBits(packet, sqn, scale, long_timeout);
}

int PacketManager::unpack(std::vector<Packet> packets, byte_t *output, int output_len) {
    std::vector<bit_t> output_bit;
    unpack(std::move(packets), output_bit);

    memset(output, 0, output_len);

    for (int i = 0; i < output_bit.size(); i++) {
        if (output_bit[i]) {
            output[i / 8] |= (1 << (i % 8));
            _log->trace("Data bit {0:3}/{2:3}: {1}", i, 1, output_bit.size());
        } else {
            _log->trace("Data bit {0:3}/{2:3}: {1}", i, 0, output_bit.size());
        }
    }

    return 1;
}

int PacketManager::unpack(std::vector<Packet> packets, byte_t **output) {
    std::vector<bit_t> output_bit;
    unpack(std::move(packets), output_bit);

    size_t bytes = 1 + (output_bit.size() - 1) / 8;
    (*output) = (byte_t *) malloc(bytes + 1);
    memset(*output, 0, bytes + 1);

    for (int i = 0; i < output_bit.size(); i++) {
        if (output_bit[i]) {
            (*output)[i / 8] |= (1 << (i % 8));
            _log->trace("Data bit {0:3}/{2:3}: {1}", i, 1, output_bit.size());
        } else {
            _log->trace("Data bit {0:3}/{2:3}: {1}", i, 0, output_bit.size());
        }
    }

    return bytes + 1;
}

// Use PacketManager::check() to verify the integrity of the packets before using this function
int PacketManager::unpack(std::vector<Packet> packets, std::vector<bit_t> &output) {


    output.clear();

    for (auto &packet : packets) {
        std::vector<bit_t> tmp(packet.getData());
        output.insert(output.end(), tmp.begin(), tmp.end());
    }

    return output.size();
}

void PacketManager::wait(int cycle_count) {
    // TODO: get cycletime from sensor
    _sens->wait(cycle_count);
}

/**************************
 * BIT BASED PRIVATE FUNCTIONS
***************************/

int PacketManager::waitForRequestBits(byte_t *sqn_had) {
    int bit;
    int had_bitsize = _ecc->getEncodedSize(P_SQN_BITS);

    *sqn_had = 0;

    for (int i = 0; i < had_bitsize; i++) {
        bit = _sens->readBit(i!=0, 0);
        if(bit < 0)
            return bit; // error

        if (bit) {
            *sqn_had |= (1 << i);
        }
    }

    _log->debug("Received sqnHad: 0x{0:2x}", *sqn_had);

    return 0;
}

int PacketManager::checkForRequestBits(byte_t *sqn_had, bool long_timeout) {
    int bit;

    *sqn_had = 0;
    int had_bitsize = _ecc->getEncodedSize(P_SQN_BITS);

    for (int i = 0; i < had_bitsize; i++) {
        bit = _sens->readBit(i, long_timeout ? 3 : 2);
        if(bit < 0)
            return bit; // error

        if (bit) {
            *sqn_had |= (1 << i);
        }
    }

    _log->debug("Received sqnHad: 0x{0:2x}", *sqn_had);

    return 0;
}

int PacketManager::requestBits(int sqn) {
    int mod_sqn = MAX_SQN + 1;
    size_t sqn_bits = P_SQN_BITS;

    byte_t sqn_had;
    byte_t sqn_byte = (byte_t) (sqn % mod_sqn & 0xFF);
    int had_bitsize = _ecc->encode(&sqn_byte, sqn_bits, &sqn_had);

    _log->info("Requesting packet {0}", sqn);
    _log->debug("{0} bit hadamard encoded sqn: 0x{1:2x}", had_bitsize, sqn_had);

    if (had_bitsize < 0) {
        return -1;
    }

    for (int i = 0; i < had_bitsize; i++) {
        _sens->sendBit((bit_t) (sqn_had & (1 << (i % 8))));
    }

    return 0;
}

int PacketManager::sendBits(Packet packet) {
    _log->debug("Sending {}bit packet.", packet.size());
    for (int i = 0; i < packet.size(); i++) {
        _sens->sendBit(packet[i]);
    }

    // debug log
    packet.printContents();
    return 0;
}

int PacketManager::receiveBits(Packet &packet, int sqn, int scale, int long_timeout) {
    int bit;

    // packet size for normal packets
    size_t packet_bitsize =
            1 + P_DATA_BITS[scale] + P_SQN_BITS + _edc->calcOutputSize(P_DATA_BITS[scale] + P_SQN_BITS + 1);
    _log->debug("Expecting {}bit packet.", packet_bitsize);

    // clean up
    std::vector<bit_t> tmp;
    for (int i = 0; i < packet_bitsize; i++) {
        bit = _sens->readBit(i, long_timeout ? 3 : 2);
        if(bit < 0)
            return bit; // error

        if (bit) {
            if (tmp.empty()) {
                // packet size for command packets
                Berger edc;
                packet_bitsize = 1 + P_CMD_BITS + P_SQN_BITS + edc.calcOutputSize(P_CMD_BITS + P_SQN_BITS + 1);
            }

            tmp.push_back(1);
        } else {
            tmp.push_back(0);
        }
    }

    packet.fromBits(tmp, scale);

    // debug log
    packet.printContents();

    // check packet, if packet is ok return 0, else return -1
    return check(packet, sqn);
}
/**************************
 * BYTE BASED PRIVATE FUNCTIONS
***************************/

int PacketManager::waitForRequestByte(byte_t *sqn_had) {
    //TODO: support for multiple bytes?

    int byte = _sens->readByte();

    if(byte < 0)
        return byte;

    *sqn_had = static_cast<byte_t >(byte & 0xFF);

    _log->debug("Received sqnHad: 0x{0:2x}", *sqn_had);

    return 0;
}

int PacketManager::checkForRequestByte(byte_t *sqn_had, bool long_timeout) {
    return waitForRequestByte(sqn_had);
}

int PacketManager::requestBytes(int sqn) {
    int mod_sqn = MAX_SQN + 1;

    byte_t sqn_had;
    byte_t sqn_byte = (byte_t) (sqn % mod_sqn & 0xFF);
    int had_bitsize = _ecc->encode(&sqn_byte, P_SQN_BITS, &sqn_had);

    _log->info("Requesting packet {0}", sqn);
    _log->debug("{0} bit hadamard encoded sqn: 0x{1:2x}", had_bitsize, sqn_had);

    if (had_bitsize < 0 || had_bitsize > 8) {
        return -1;
    }

    //_sens->readByte();
    _sens->sendByte(sqn_had);

    //TODO: error handling

    return 0;
}

int PacketManager::sendBytes(Packet packet) {
    //TODO: implement
    _log->debug("Sending {}bit packet.", packet.size());

    int bitsize = packet.size();
    int bytesize = (bitsize-1)/7 + 1;
    std::vector<bit_t> tmp;
    packet.toBits(tmp);
    std::vector<byte_t> tmp2;
    byte_t  tmp3 = 0x00;

    int bytes = 1 + (tmp.size() - 1) / 7;

    for (int i = 0; i < bytes * 7; i++) {

        if (i < tmp.size() && tmp[i]) {
            tmp3 |= (1 << (i % 7));
        }

        if((i+1)%7==0){
            tmp2.push_back(tmp3);
            tmp3 = 0x00;
        }
    }

    for (int i = 0; i < bytesize; i++) {
        _sens->sendByte(tmp2[i]);
        _sens->readByte(); // wait for ack
    }

    // debug log
    packet.printContents();
    return 0;
}

int PacketManager::receiveBytes(Packet &packet, int sqn, int scale, int long_timeout) {
    //TODO: implement
    int byte;

    // packet size for normal packets
    int packet_bitsize =
            1 + P_DATA_BITS[scale] + P_SQN_BITS + _edc->calcOutputSize(P_DATA_BITS[scale] + P_SQN_BITS + 1);

    int packet_bytesize = (packet_bitsize-1) / 7 + 1;
    _log->debug("Expecting {}byte packet.", packet_bytesize);

    std::vector<byte_t> tmp;
    std::vector<bit_t> tmp2;
    for (int i = 0; i < packet_bytesize; i++) {
        byte = _sens->readByte();
        if(byte < 0)
            return byte; // error

        //write ack
        int temp = ~byte;
        _sens->sendByte((unsigned char)(temp & 0x7F));

        tmp.push_back((unsigned char)byte);

        if(i == 0 && byte & 0x01){
            packet_bytesize = 2;
        }
    }

    for (int i = 0; i < tmp.size(); i++) {
        for (int l = 0; l < 7; l++) {
            tmp2.push_back((unsigned char)((tmp[i] & (1 << l)) >> l));
        }
    }

    for(int i=0; i<tmp.size(); i++){
        _log->trace("byte: {0:x}", tmp[i]);
    }

    for(int i=0; i<tmp2.size(); i++){
        _log->trace("bit: {0:x}", tmp2[i]);
    }


    packet.fromBits(tmp2, scale);

    // debug log
    packet.printContents();

    // check packet, if packet is ok return 0, else return -1
    return check(packet, sqn);
}

/**************************
 * GENERAL PRIVATE FUNCTIONS
***************************/

int PacketManager::check(Packet packet, int sqn) {
    // check in main?
    if (packet.isValid() && packet.hasSqn(sqn)) {
        _log->debug("Packet ok");
        return 0;
    }

    return -1;
}

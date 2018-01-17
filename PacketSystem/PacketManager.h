#pragma once

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <bitset>

#include "../spdlog/spdlog.h"

#include "../Sensors/HTS221.h"
#include "../Defines.h"
#include "ECC/ECC.h"
#include "EDC/EDC.h"
#include "Packet.h"
#include "PacketFactory.h"

namespace spd = spdlog;

class PacketManager {
public:
    PacketManager();
    ~PacketManager();

    /**
     * Wait until someone requests a packet
     * @param sqnHad: Hadamard encoded sequence number
     * @return: TODO: dummy
     **/
    int waitForRequest(byte *sqnHad);

    /**
     * Check if someone requests a packet (waits for a maximum of XX cycles)
     * @param sqnHad: Hadamard encoded sequence number
     * @return: TODO: dummy
     **/
    int checkForRequest(byte *sqnHad, int timeout);

    /**
     * Request a packet using the sequence number sqn
     * @param sqn: sequence number
     * @return: TODO: dummy
     **/
    int request(unsigned int sqn);

    int checkRequest(byte *sqn_had);

    /**
     * Send a packet over the covert channel
     * @param packet: packet to send
     * @return: 0 on success, -1 on error
     **/
    int send(Packet packet);

    /**
     * Receive a packet and check its sequence number
     * @param packet: packet buffer
     * @param sqn: sequence number
     * @return: 0 on success, -1 on timeout (no data), -2 on timeout (not enough data),
     **/
    int receive(Packet &packet, int sqn, int scale, int timeout);

    /**
     * Unpacks a packet-stream (must be a valid packet)
     * @param packets: packet buffer
     * @param numPackets: number of packets in the buffer
     * @param bytesPerPacket: size per packet in bytes
     * @param output: buffer for the unpacked data
     * @return: output length in bytes
     **/
    int unpack(std::vector<Packet> packets, byte **output);

    int unpack(std::vector<Packet> packets, std::vector<bit_t> &output);

    void printInfo();

    void wait(int cycle_count);

private:
    ECC *m_ECC;
    EDC *m_EDC;
    Sensor *m_sens;
    std::shared_ptr<spd::logger> m_log;

    int scale;

    int check(Packet packet, int sqn);
};

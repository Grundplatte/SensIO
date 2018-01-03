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
    int checkForRequest(byte *sqnHad);

    /**
     * Request a packet using the sequence number sqn
     * @param sqn: sequence number
     * @return: TODO: dummy
     **/
    int request(unsigned int sqn);

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
    int receive(Packet &packet, int sqn);

    /**
     * Unpacks a packet-stream (must be a valid packet)
     * @param packets: packet buffer
     * @param numPackets: number of packets in the buffer
     * @param bytesPerPacket: size per packet in bytes
     * @param output: buffer for the unpacked data
     * @return: output length in bytes
     **/
// TODO: change to bitset
    int unpack(std::vector<Packet> packets, byte **output);

    int unpack(std::vector<Packet> packets, std::vector<bit_t> &output);

    /**
     * Packs data into a packet-stream
     * @param data: input data buffer
     * @param length: input length in bytes
     * @param output: packet buffer
     * @return: number of packets
     **/
    int pack(const byte *data, unsigned int length, std::vector<Packet> &output);

    int pack(std::vector<bit_t> data, std::vector<Packet> &output);

    void insertCommandPacket(std::vector<Packet> &packets, int cmd, int sqn);

    void printInfo();

    void wait(int cycleCount);

    void scaleUp();

    void scaleDown();

private:
    // maximum of 15 bits supported for berger codes
    ECC *m_ECC;
    EDC *m_EDC;
    Sensor *m_sens;
    std::shared_ptr<spd::logger> m_log;


    int packet_scale = 2;
    std::vector<int> packet_scales;
    int check(Packet packet, int sqn);
};

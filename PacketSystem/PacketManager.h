#pragma once

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <bitset>

#include "../spdlog/spdlog.h"

#include "../Defines.h"
#include "ECC/ECC.h"
#include "EDC/EDC.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "../Sensors/Sensor.h"

namespace spd = spdlog;

class PacketManager {
public:
    PacketManager(std::shared_ptr<ECC> ecc, std::shared_ptr<EDC> edc, std::shared_ptr<Sensor> sensor);

    /**
     * Wait until someone requests a packet (no initial timeout)
     * @param sqn_had: Hadamard encoded sequence number
     * @return: return 0 on success, -2 on timeout while receiving
     **/
    int waitForRequest(byte *sqn_had);

    /**
     * Check if someone requests a packet
     * @param sqn_had: Hadamard encoded sequence number
     * @return: return 0 on success, -1 on initial timeout, -2 on timeout while receiving
     **/
    int checkForRequest(byte *sqn_had, int long_timeout);

    /**
     * Request a packet using the sequence number sqn
     * @param sqn: sequence number
     * @return: return 0 on success, -1 on error
     **/
    int request(int sqn);

    /**
     * Request a packet using the sequence number sqn
     * @param sqn: sequence number
     * @return: number of bits in sqn_had on success, -1 on error
     **/
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
     * @param long_timeout: use a longer timeout (for retransmissions)
     * @return: 0 on success, -1 on timeout (no data), -2 on timeout (not enough data),
     **/
    int receive(Packet &packet, int sqn, int scale, int long_timeout);

    int unpack(std::vector<Packet> packets, byte *output, int output_len);

    /**
     * Unpacks a packet-stream (must be a valid packet)
     * @param packets: collection of packets
     * @param output: buffer for the unpacked data
     * @return: output length in bytes
     **/
    int unpack(std::vector<Packet> packets, byte **output);

    /**
     * Unpacks a packet-stream (must be a valid packet)
     * @param packets: collection of packets
     * @param output: collection of databits
     * @return: output length in bit
     **/
    int unpack(std::vector<Packet> packets, std::vector<bit_t> &output);

    /**
     * Wait a specified number of cycles
     * @param cycle_count: Number of cycles
     **/
    void wait(int cycle_count);

private:
    std::shared_ptr<ECC> _ecc;
    std::shared_ptr<EDC> _edc;
    std::shared_ptr<Sensor> _sens;
    std::shared_ptr<spd::logger> _log;

    int _scale = P_INIT_SCALE;

    int check(Packet packet, int sqn);
};

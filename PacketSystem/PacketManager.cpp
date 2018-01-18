#include <cmath>
#include <utility>
#include <sstream>
#include <iostream>
#include "PacketManager.h"
#include "ECC/Hadamard.h"
#include "EDC/Berger.h"

namespace spd = spdlog;

PacketManager::PacketManager() {
    // <-- Error correction code settings (for the sequence number) -->
    _ecc = new Hadamard();
    //m_ECC = new NoECC();

    // <-- Error detection code settings (for the packet)-->
    _edc = new Berger();
    //m_EDC = new NoEDC();

    // <-- Sensor settings -->
    _sens = new HTS221();
    if (!_sens->isActive())
        _sens->toggleOnOff(1);

    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("PMgr");
    _log = log ? log : spd::stdout_color_mt("PMgr");
}

PacketManager::~PacketManager() {
    delete (_edc);
    delete (_ecc);
    delete (_sens);
}

int PacketManager::waitForRequest(byte *sqn_had) {
    int bit;
    struct timespec start{}, stop{};
    double accum;

    *sqn_had = 0;
    int had_bitsize = _ecc->getEncodedSize(P_SQN_BITS);

    for (int i = 0; i < had_bitsize; i++) {

        _sens->waitForSensReady();
        clock_gettime(CLOCK_REALTIME, &start);
        // wait until someone accesses the sensor results
        do {
            bit = _sens->tryReadBit();

            clock_gettime(CLOCK_REALTIME, &stop);
            accum = stop.tv_nsec - start.tv_nsec;
            // rollover
            if (accum < 0) {
                accum += 1000000000;
            }

            // timeout > delay ms (only relevant in transmission has already started)
            if (i != 0 && accum > CYCLE_DELAY) {
                _log->warn("[D] Timeout while receiving a sequence number ({0} bits)", i);
                return -2;
            }
        } while (bit < 0);

        if (bit) {
            *sqn_had |= (1 << i);
        }
    }

    _log->debug("Received sqnHad: 0x{0:2x}", *sqn_had);

    return 0;
}

int PacketManager::checkForRequest(byte *sqn_had, int long_timeout) {
    int bit;
    struct timespec start{}, stop{};
    double accum;
    int timeout = long_timeout ? CYCLE_DELAY * 3 : CYCLE_DELAY * 2;

    *sqn_had = 0;
    int had_bitsize = _ecc->getEncodedSize(P_SQN_BITS);

    for (int i = 0; i < had_bitsize; i++) {

        _sens->waitForSensReady();
        clock_gettime(CLOCK_REALTIME, &start);
        // wait until someone accesses the sensor results
        do {
            bit = _sens->tryReadBit();
            //printf("[D] receive: bit = %i\n", bit);

            clock_gettime(CLOCK_REALTIME, &stop);
            accum = stop.tv_nsec - start.tv_nsec;
            // rollover
            if (accum < 0) {
                accum += 1000000000;
            }

            // timeout > delay ms (only relevant in transmission has already started)
            if (i != 0 && accum > CYCLE_DELAY) {
                _log->warn("Timeout while receiving a sequence number ({0} bits)", i);
                return -1;
            } else if (accum > timeout) {
                _log->warn("Timeout while waiting for a sequence number");
                return -2;
            }
        } while (bit < 0);

        if (bit)
            *sqn_had |= (1 << i);
    }

    _log->debug("Received sqnHad: 0x{0:2x}", *sqn_had);

    return 0;
}

int PacketManager::request(int sqn) {
    int mod_sqn = MAX_SQN + 1;
    size_t sqn_bits = P_SQN_BITS;

    struct timespec req{}, rem{};
    req.tv_sec = 0;
    req.tv_nsec = 40000000; // 160ms

    byte sqn_had;
    byte sqn_byte = (byte) (sqn % mod_sqn & 0xFF);
    int had_bitsize = _ecc->encode(&sqn_byte, sqn_bits, &sqn_had);

    _log->info("Requesting packet {0}", sqn);
    _log->debug("{0} bit hadamard encoded sqn: 0x{1:2x}", had_bitsize, sqn_had);

    if (had_bitsize < 0) {
        return -1;
    }

    for (int i = 0; i < had_bitsize; i++) {
        // wait until the sensor is ready
        _sens->waitForSensReady();
        nanosleep(&req, &rem);
        _sens->sendBit((bit_t) (sqn_had & (1 << (i % 8))));
    }

    return 0;
}

int PacketManager::unpack(std::vector<Packet> packets, byte **output) {
    std::vector<bit_t> output_bit;
    unpack(std::move(packets), output_bit);

    size_t bytes = 1 + (output_bit.size() - 1) / 8;
    (*output) = (byte *) malloc(bytes + 1);
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

// send packet
int PacketManager::send(Packet packet) {
    struct timespec req{}, rem{};
    req.tv_sec = 0;
    req.tv_nsec = WRITE_DELAY;

    _log->debug("Sending {}bit packet.", packet.size());
    for (int i = 0; i < packet.size(); i++) {

        // wait until the sensor is ready
        _sens->waitForSensReady();
        nanosleep(&req, &rem);
        _sens->sendBit(packet[i]);
    }

    // debug log
    packet.printContents();
    return 0;
}

int PacketManager::receive(Packet &packet, int sqn, int scale, int long_timeout) {
    int bit;
    struct timespec start{}, stop{};
    double accum;
    int timeout = long_timeout ? CYCLE_DELAY * 3 : CYCLE_DELAY * 2;

    // packet size for normal packets
    size_t packet_bitsize =
            1 + P_DATA_BITS[scale] + P_SQN_BITS + _edc->calcOutputSize(P_DATA_BITS[scale] + P_SQN_BITS + 1);
    _log->debug("Expecting {}bit packet.", packet_bitsize);

    // clean up
    std::vector<bit_t> tmp;
    for (int i = 0; i < packet_bitsize; i++) {
        _sens->waitForSensReady();
        clock_gettime(CLOCK_REALTIME, &start);

        // wait until someone accesses the sensor results
        do {
            bit = _sens->tryReadBit();

            clock_gettime(CLOCK_REALTIME, &stop);
            accum = stop.tv_nsec - start.tv_nsec;
            // rollover
            if (accum < 0) {
                accum += 1000000000;
            }

            // timeout > delay ms
            if (i != 0 && accum > CYCLE_DELAY) {
                _log->warn("Timeout while receiving, trying again");
                return -1;
            } else if (accum > timeout) {
                _log->warn("Sender didnt start the transmission");
                return -2;
            }
        } while (bit < 0);

        if (bit) {
            if (tmp.empty()) {
                // packet size for command packets
                packet_bitsize = 1 + P_CMD_BITS + P_SQN_BITS + _edc->calcOutputSize(P_CMD_BITS + P_SQN_BITS + 1);
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

int PacketManager::check(Packet packet, int sqn) {
    // check in main?
    if (packet.isValid() && packet.hasSqn(sqn)) {
        _log->debug("Packet ok");
        return 0;
    }

    return -1;
}

void PacketManager::wait(int cycle_count) {
    // TODO: get cycletime from sensor
    struct timespec req{}, rem{};
    req.tv_sec = 0;
    req.tv_nsec = (__syscall_slong_t) CYCLE_DELAY * cycle_count; // 80ms per cycle

    nanosleep(&req, &rem);
}

int PacketManager::checkRequest(byte *sqn_had) {
    Hadamard ecc = Hadamard();
    size_t had_length = (size_t) ecc.getEncodedSize(P_SQN_BITS);
    // error correction
    // return ecc.decode(sqnHad, hda_length, sqn)

    // error detection
    return ecc.check(sqn_had, had_length);
}

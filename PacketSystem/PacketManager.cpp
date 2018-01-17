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
    m_ECC = new Hadamard();
    //m_ECC = new NoECC();

    // <-- Error detection code settings (for the packet)-->
    m_EDC = new Berger();
    //m_EDC = new NoEDC();

    // <-- Sensor settings -->
    m_sens = new HTS221();
    if (!m_sens->isActive())
        m_sens->toggleOnOff(1);

    scale = 2;

    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("PMgr");
    m_log = log ? log : spd::stdout_color_mt("PMgr");
}

PacketManager::~PacketManager() {
    delete (m_EDC);
    delete (m_ECC);
    delete (m_sens);
}

// TODO: make dynamic!
int PacketManager::waitForRequest(byte *sqnHad) {
    int i, bit, hadBitSize;
    struct timespec start, stop;
    double accum;

    *sqnHad = 0;
    hadBitSize = m_ECC->getEncodedSize(P_SQN_BITS);
    for (i = 0; i < hadBitSize; i++) {

        m_sens->waitForSensReady();
        clock_gettime(CLOCK_REALTIME, &start);
        // wait until someone accesses the sensor results
        do {
            bit = m_sens->tryReadBit();
            //printf("[D] receive: bit = %i\n", bit);

            clock_gettime(CLOCK_REALTIME, &stop);
            accum = stop.tv_nsec - start.tv_nsec;
            // rollover
            if (accum < 0) {
                accum += 1000000000;
            }

            // timeout > delay ms (only relevant in transmission has already started)
            if (i != 0 && accum > P_MAXDELAY) {
                m_log->warn("[D] Timeout while receiving a sequence number ({0} bits)", i);
                return -2;
            }
        } while (bit < 0);

        if (bit) {
            *sqnHad |= (1 << i);
        }
    }

    m_log->debug("Received sqnHad: 0x{0:2x}", *sqnHad);

    return 0;
}

int PacketManager::checkForRequest(byte *sqnHad) {
    int i, bit, hadBitSize;
    struct timespec start, stop;
    double accum;

    *sqnHad = 0;
    hadBitSize = m_ECC->getEncodedSize(P_SQN_BITS);
    for (i = 0; i < hadBitSize; i++) {

        m_sens->waitForSensReady();
        clock_gettime(CLOCK_REALTIME, &start);
        // wait until someone accesses the sensor results
        do {
            bit = m_sens->tryReadBit();
            //printf("[D] receive: bit = %i\n", bit);

            clock_gettime(CLOCK_REALTIME, &stop);
            accum = stop.tv_nsec - start.tv_nsec;
            // rollover
            if (accum < 0) {
                accum += 1000000000;
            }

            // timeout > delay ms (only relevant in transmission has already started)
            if (accum > P_MAXDELAY) {
                if (i == 0) {
                    m_log->warn("Timeout while waiting for a sequence number");
                    return -2;
                } else {
                    m_log->warn("Timeout while receiving a sequence number ({0} bits)", i);
                    return -1;
                }
            }
        } while (bit < 0);

        if (bit) {
            *sqnHad |= (1 << i);
            //printf("[D] receive: received 1\n");
        } else {
            //printf("[D] receive: received 0\n");
        }
    }

    m_log->debug("Received sqnHad: 0x{0:2x}", *sqnHad);

    return 0;
}

// TODO: support more than 3/8bit
int PacketManager::request(unsigned int sqn) {
    int i, hadBitSize, sqn_bits;
    struct timespec req, rem;
    byte sqnByte, sqnHad;
    unsigned int modSqn = (unsigned int) MAX_SQN + 1;

    sqn_bits = P_SQN_BITS;
    req.tv_sec = 0;
    req.tv_nsec = 10000000; // 160ms

    sqnByte = (byte) (sqn % modSqn & 0xFF);
    hadBitSize = m_ECC->encode(&sqnByte, sqn_bits, &sqnHad); //Hadamard implementation only supports 3bits

    m_log->info("Requesting packet {0}", sqn);
    m_log->debug("{0} bit hadamard encoded sqn: 0x{1:2x}", hadBitSize, sqnHad);

    if (hadBitSize < 0) {
        return -1;
    }

    m_sens->waitForSensReady();
    nanosleep(&req, &rem);

    for (i = 0; i < hadBitSize; i++) {
        // wait until the sensor is ready
        m_sens->waitForSensReady();
        m_sens->sendBit((bit_t) (sqnHad & (1 << (i % 8))));
    }

    return 0;
}

/*
int PacketManager::pack(const byte *data, unsigned int length, std::vector<Packet> &output) {

    if (length == 0)
        return -1;

    std::vector<bit_t> data_bit;

    // convert byte data to bits
    for (int i = 0; i < length; i++) {
        for (int l = 0; l < 8; l++) {
            data_bit.push_back(((data[i] & (1 << l)) >> l));
            m_log->trace("Data bit {}: {}", i * 8 + l, data_bit[i * 8 + l]);
        }
    }

    return pack(data_bit, output);
}
// send one packet (12bit data, 3bit sqn, 4bit edc = 19bit)
// memory is allocated inside the function, must be freed by caller after use

int PacketManager::pack(std::vector<bit_t> data, std::vector<Packet> &output)
{
    output.clear();

    m_log->debug("Packing {0} bit", data.size());

    // split into P_DATA_BITS bit packets
    int packet_sqn = 0;
    int global_i = 0;
    for (global_i = 0; global_i + P_DATA_BITS < data.size(); global_i += P_DATA_BITS) {
        std::vector<bit_t> packetdata(data.begin() + global_i, data.begin() + global_i + P_DATA_BITS);
        Packet tmp = Packet(packetdata, packet_sqn++);
        output.push_back(tmp);
    }
    // last packet may be smaller
    if (global_i < data.size() - 1) {
        std::vector<bit_t> packetdata(data.begin() + global_i, data.end());
        Packet tmp = Packet(packetdata, packet_sqn++);
        output.push_back(tmp);
    }

    insertCommandPacket(output, Packet::CMD_STOP, packet_sqn);

    m_log->debug("Got {0} packets with {1} bit each", output.size(), output.front().size());
}
*/
int PacketManager::unpack(std::vector<Packet> packets, byte **output) {
    std::vector<bit_t> output_bit;
    unpack(std::move(packets), output_bit);

    size_t bytes = 1 + (output_bit.size() - 1) / 8;
    (*output) = (byte *) malloc(bytes + 1);
    memset(*output, 0, bytes + 1);

    for (int i = 0; i < output_bit.size(); i++) {
        if (output_bit[i]) {
            (*output)[i / 8] |= (1 << (i % 8));
            m_log->trace("Data bit {0:3}/{2:3}: {1}", i, 1, output_bit.size());
        } else {
            m_log->trace("Data bit {0:3}/{2:3}: {1}", i, 0, output_bit.size());
        }
    }
}

// Use PacketManager::check() to verify the integrity of the packets before using this function
int PacketManager::unpack(std::vector<Packet> packets, std::vector<bit_t> &output) {

    output.clear();

    for (int i = 0; i < packets.size(); i++) {
        std::vector<bit_t> tmp(packets[i].getData());
        output.insert(output.end(), tmp.begin(), tmp.end());
    }

    return output.size();
}

// send packet
int PacketManager::send(Packet packet) {
    struct timespec req, rem;
    req.tv_sec = 0;
    req.tv_nsec = 10000000; // 10ms

    m_sens->waitForSensReady();
    nanosleep(&req, &rem);

    m_log->debug("Sending {}bit packet.", packet.size());
    for (int i = 0; i < packet.size(); i++) {
        //printf("[D] Sending bit %i/%i\n", i, packetBitSize);

        // wait until the sensor is ready
        m_sens->waitForSensReady();
        m_sens->sendBit(packet[i]);
    }

    // debug log
    packet.printContents();
    return 0;
}

int PacketManager::receive(Packet &packet, int sqn, int scale) {
    int bit;
    struct timespec start, stop;
    double accum;

    // packet size for normal packets
    size_t packetBitSize =
            1 + P_DATA_BITS[scale] + P_SQN_BITS + m_EDC->calcOutputSize(P_DATA_BITS[scale] + P_SQN_BITS + 1);
    m_log->debug("Expecting {}bit packet.", packetBitSize);

    // clean up
    std::vector<bit_t> tmp;
    for (int i = 0; i < packetBitSize; i++) {
        m_sens->waitForSensReady();
        clock_gettime(CLOCK_REALTIME, &start);

        //m_log->debug("Receiving bit {0}/{1}", i, packetBitSize);
        // wait until someone accesses the sensor results
        do {
            bit = m_sens->tryReadBit();
            //printf("[D] receive: bit = 0x%02x\n", bit);

            clock_gettime(CLOCK_REALTIME, &stop);
            accum = stop.tv_nsec - start.tv_nsec;
            // rollover
            if (accum < 0) {
                accum += 1000000000;
            }

            // timeout > delay ms
            if (accum > P_MAXDELAY) {
                if (i == 0) {
                    m_log->warn("Sender didnt start the transmission");
                    return -2; // sender didnt start the transmission
                } else {
                    m_log->warn("Timeout while receiving, trying again");
                    return -1; // timeout (desync? => restart receive)
                }
            }
        } while (bit < 0);

        if (bit) {
            if (tmp.empty()) {
                // packet size for command packets
                packetBitSize = 1 + P_CMD_BITS + P_SQN_BITS + m_EDC->calcOutputSize(P_CMD_BITS + P_SQN_BITS + 1);
            }

            tmp.push_back(true);
        } else {
            tmp.push_back(false);
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
    if (packet.isValid()) { //} && packet.hasSqn(sqn)){
        m_log->debug("Packet ok");
        return 0;
    }

    return -1;
}

void PacketManager::printInfo() {
    int sqn_bits = P_SQN_BITS;
    int data_bits = P_DATA_BITS[scale];

    m_log->debug("===== REQUEST =====");
    m_log->debug("= SQN: {0:2}bits => max sqn: {1}", sqn_bits, MAX_SQN);
    m_log->debug("= Encoded: {0:2}bits", m_ECC->getEncodedSize(sqn_bits));
    m_log->debug("===================");
    m_log->debug("");
    m_log->debug("====== PACKET ======");
    m_log->debug("= DATA: {0:2}bit", data_bits);
    m_log->debug("= SQN : {0:2}bit", sqn_bits);
    m_log->debug("= EDC : {0:2}bit", m_EDC->calcOutputSize(P_DATA_BITS[scale] + P_SQN_BITS + 1));
    m_log->debug("====================");
    m_log->debug("");
}

void PacketManager::wait(int cycle_count) {
    // TODO: get cycletime from sensor
    struct timespec req, rem;

    req.tv_sec = 0;
    req.tv_nsec = MAXDELAY_MS / 2 * cycle_count; // 80ms per cycle

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

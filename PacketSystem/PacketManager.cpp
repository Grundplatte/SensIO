#include <cmath>
#include <utility>
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
    hadBitSize = m_ECC->getEncodedSize();
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
            if (i != 0 && accum > (MAXDELAY % 1000) * 1000000) {
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
    hadBitSize = m_ECC->getEncodedSize();
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
            if (accum > (MAXDELAY % 1000) * 1000000) {
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
    unsigned int modSqn = (unsigned int) getMaxSqn() + 1;

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

int PacketManager::pack(const byte *data, unsigned int length, std::vector<std::vector<bit_t> > &output) {

    if (length == 0)
        return -1;

    std::vector<bit_t> data_bit;

    // convert byte data to bits
    for (int i = 0; i < length; i++) {
        for (int l = 0; l < 8; l++) {
            data_bit.push_back((data[i] & (1 << l)));
        }
    }

    return pack(data_bit, output);
}
// send one packet (12bit data, 3bit sqn, 4bit edc = 19bit)
// memory is allocated inside the function, must be freed by caller after use
int PacketManager::pack(std::vector<bit_t> data, std::vector<std::vector<bit_t> > &output)
{
    output.clear();

    int modSqn = getMaxSqn() + 1;

    m_log->debug("Packing {0} bit", data.size());

    // split into P_DATA_BITS bit packets
    int global_i = 0;
    int packet_sqn = 0;
    while (global_i < data.size()) {
        std::vector<bit_t> temp;
        std::vector<bit_t> temp_enc;

        // data bits
        for (int local_i = 0; local_i < P_DATA_BITS; local_i++) {
            temp.push_back(data[global_i]);
            global_i++;

            // fill last packet with zeros
            // TODO: better solution
            if (global_i >= data.size()) {
                int missing = P_DATA_BITS - temp.size();
                for (int i = 0; i < missing; i++) {
                    temp.push_back(false);
                }
                break;
            }
        }

        // add sqn bits
        for (int local_i = 0; local_i < P_SQN_BITS; local_i++) {
            temp.push_back((bit_t) ((packet_sqn % modSqn) & (1 << local_i)));
        }
        packet_sqn++;

        // add EDC bits
        m_EDC->generate(temp, temp_enc);

        // packet complete => add to output array
        output.push_back(temp_enc);
    }

    m_log->debug("Got {0} packets with {1} bit each", output.size(), output.front().size());
}

int PacketManager::unpack(std::vector<std::vector<bit_t> > packets, byte **output) {
    std::vector<bit_t> output_bit;
    unpack(std::move(packets), output_bit);

    size_t bytes = 1 + (output_bit.size() - 1) / 8;
    (*output) = (byte *) malloc(bytes);

    // FIXME: put in warning that other people will understand
    m_log->warn("Data not byte sized.");

    for (int i = 0; i < output_bit.size(); i++) {
        (*output)[i / 8] |= (1 << (i % 8));
    }
}

// Use PacketManager::check() to verify the integrity of the packets before using this function
int PacketManager::unpack(std::vector<std::vector<bit_t> > packets, std::vector<bit_t> &output) {

    output.clear();

    for (auto &packet : packets) {
        for (int l = 0; l < P_DATA_BITS; l++) {
            output.push_back(packet[l]);
        }
    }

    return output.size();
}

// send packet
int PacketManager::send(std::vector<bit_t> packet) {
    struct timespec req, rem;
    req.tv_sec = 0;
    req.tv_nsec = 10000000; // 100ms TODO: change to smaller value

    m_sens->waitForSensReady();
    nanosleep(&req, &rem);

    for (int i = 0; i < packet.size(); i++) {
        //printf("[D] Sending bit %i/%i\n", i, packetBitSize);

        // wait until the sensor is ready
        m_sens->waitForSensReady();
        m_sens->sendBit(packet[i]);
    }

    return 0;
}

int PacketManager::receive(std::vector<bit_t> &packet, unsigned int sqn) {
    int bit;
    struct timespec start, stop;
    double accum;
    size_t packetBitSize = P_DATA_BITS + P_SQN_BITS + m_EDC->calcOutputSize(P_DATA_BITS + P_SQN_BITS);

    // clean up
    packet.clear();

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
            if (accum > (MAXDELAY % 1000) * 1000000) {
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
            packet.push_back(true);
        } else {
            packet.push_back(false);
        }
    }

    // check packet, if packet is ok return 0, else return -1
    return check(packet, sqn);
}

int PacketManager::check(std::vector<bit_t> packet, int sqn) {
    // check edc
    if (m_EDC->check(packet, P_DATA_BITS + P_SQN_BITS) < 0) {
        // invalid
        return -1;
    }

    // check sqn
    // TODO: check it!
    int sqn_pack = 0;
    for (int i = 0; i < P_SQN_BITS; i++) {
        sqn_pack |= (packet[P_DATA_BITS + i] << i);
    }

    if (sqn_pack != sqn)
        return -2;

    return 0;
    //m_log->debug("P: 0x{0:2x} 0x{1:2x} 0x{2:2x}", packet[0], packet[1], packet[2]);
    //m_log->debug("Berger: 0x{0:2x} 0x{1:2x}", berger_pack, berger_calc);
}

int PacketManager::getMaxSqn() {
    return (int) pow(2, P_SQN_BITS) - 1;
}

int PacketManager::getPacketSizeInBits() {
    return P_DATA_BITS + P_SQN_BITS + m_EDC->calcOutputSize(P_DATA_BITS + P_SQN_BITS);
}

void PacketManager::printInfo() {
    int sqn_bits = P_SQN_BITS;
    int data_bits = P_DATA_BITS;

    m_log->debug("===== REQUEST =====");
    m_log->debug("= SQN: {0:2}bits => max sqn: {1}", sqn_bits, getMaxSqn());
    m_log->debug("= Encoded: {0:2}bits", m_ECC->getEncodedSize());
    m_log->debug("===================");
    m_log->debug("");
    m_log->debug("====== PACKET ======");
    m_log->debug("= DATA: {0:2}bit", data_bits);
    m_log->debug("= SQN : {0:2}bit", sqn_bits);
    m_log->debug("= EDC : {0:2}bit", m_EDC->calcOutputSize(P_DATA_BITS + P_SQN_BITS));
    m_log->debug("====================");
}

void PacketManager::wait(int cycleCount) {
    // TODO: get cycletime from sensor
    struct timespec req, rem;

    req.tv_sec = 0;
    req.tv_nsec = MAXDELAY / 2 * cycleCount; // 80ms per cycle

    nanosleep(&req, &rem);
}

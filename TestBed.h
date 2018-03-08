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
    static const int ECC_NOECC = 0;
    static const int ECC_HADAMARD = 1;

    static const int EDC_NOEDC = 0;
    static const int EDC_BERGER = 1;

    static const int HAL_I2C = 0;
    static const int HAL_SPI = 1;

    static const int SENSOR_HTS221_FLAGS = 0;
    static const int SENSOR_LPS25H_UNUSED = 1;
    static const int SENSOR_HTS221_TOGGLE = 2;

    TestBed();

    void setRequestECC(int type);

    void setPacketEDC(int type);

    void setHAL(int halType);

    void setSensor(int sensorType);

    void setTestBuffer(unsigned char *data, int length);

    int runTest(bool send);

private:
    std::shared_ptr<ECC> _requestECC;// TODO: restructure edc/ecc?
    std::shared_ptr<EDC> _packetEDC;
    std::shared_ptr<HAL> _hal;
    std::shared_ptr<Sensor> _sensor;

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

    int runTestSend();

    int runTestReceive();
};


#endif //SIDECHANNEL_TESTBED_H

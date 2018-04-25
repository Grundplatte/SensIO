/**
    SensIO
    ReadFlags.h

    Implementation of the "ReadFlags" - Attack. Uses the "result ready" flags from sensors to transmit data.
    For more information see ***

    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_READFLAGS_H
#define SIDECHANNEL_READFLAGS_H


#include "AttackBase.h"

class ReadFlags : public AttackBase {
public:
    ReadFlags(std::shared_ptr<EDC> edc, std::shared_ptr<SensorBase> sensor);

    int send(Packet packet) override;

    int receive(Packet &packet, int scale, bool re_receive) override;

    int request(byte_t req) override;

    int waitForRequest(byte_t &sqn_had, bool re_receive, bool initial) override;

    void wait(int cycles) override;

private:
    int _reg_T;
    int _reg_F;

    const byte_t READY_MASK = 0x03;

    int waitForSensReady();
    int readBit(int delay);
};


#endif //SIDECHANNEL_READFLAGS_H

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
    int send(Packet packet) override;

    int receive(Packet &packet, int scale) override;

    int request(byte_t req) override;

    int waitForRequest() override;

    int wait(int cycles) override;
};


#endif //SIDECHANNEL_READFLAGS_H

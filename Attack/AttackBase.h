//
// Created by Markus Feldbacher on 11.04.18.
//

#ifndef SIDECHANNEL_ATTACKBASE_H
#define SIDECHANNEL_ATTACKBASE_H

#include "../PacketSystem/Packet.h"

class AttackBase {

public:
    virtual int send(Packet packet) = 0;
    virtual int receive(Packet &packet, int scale) = 0;
    virtual int request(byte_t req) = 0;
    virtual int waitForRequest() = 0;
    virtual int wait(int cycles) = 0;
};

#endif //SIDECHANNEL_ATTACKBASE_H

//
// Created by Markus Feldbacher on 12.04.18.
//

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

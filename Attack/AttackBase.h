//
// Created by Markus Feldbacher on 11.04.18.
//

#ifndef SIDECHANNEL_ATTACKBASE_H
#define SIDECHANNEL_ATTACKBASE_H

#include "../PacketSystem/Packet.h"
#include "../Sensors/SensorBase.h"

class AttackBase {

public:
    virtual int send(Packet packet) = 0;
    virtual int receive(Packet &packet, int scale) = 0;
    virtual int request(byte_t req) = 0;
    virtual int waitForRequest() = 0;
    virtual int wait(int cycles) = 0;

protected:
    std::shared_ptr<EDC> _edc;
    std::shared_ptr<SensorBase> _sens;
    std::shared_ptr<spd::logger> _log;
};

#endif //SIDECHANNEL_ATTACKBASE_H

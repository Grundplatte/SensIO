/**
    SensIO
    UnusedRegisters.h

    Implementation of the "UnusedRegister" - Attack. This attack uses an unused sensor register (e.g. threshold, ...)
    to transmit data.

    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_UNUSEDREGISTERS_H
#define SIDECHANNEL_UNUSEDREGISTERS_H


#include "AttackBase.h"
#include "../Sensors/SensorBase.h"

class UnusedRegisters : public AttackBase{

public:
    UnusedRegisters(std::shared_ptr<EDC> edc, std::shared_ptr<SensorBase> sensor);

    int send(Packet packet) override;

    int receive(Packet &packet, int scale) override;

    int request(byte_t req) override;

    int waitForRequest() override;

    int wait(int cycles) override;

private:
    unsigned char _last_byte = 0;
    bool _transmission = false;

    int _unused_reg_addr;

};


#endif //SIDECHANNEL_UNUSEDREGISTERS_H

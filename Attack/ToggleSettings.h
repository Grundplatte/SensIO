//
// Created by Markus Feldbacher on 12.04.18.
//

#ifndef SIDECHANNEL_TOGGLESETTINGS_H
#define SIDECHANNEL_TOGGLESETTINGS_H


#include "AttackBase.h"
#include "../Sensors/SensorBase.h"

class ToggleSettings : public AttackBase {
public:
    ToggleSettings(std::shared_ptr<EDC> edc, std::shared_ptr<SensorBase> sensor);

    int send(Packet packet) override;

    int receive(Packet &packet, int scale) override;

    int request(byte_t req) override;

    int waitForRequest() override;

    int wait(int cycles) override;

private:
    unsigned char _ref_value = 0;
    bool _last_bit = 0;
    int _setting_reg_addr;
};


#endif //SIDECHANNEL_TOGGLESETTINGS_H

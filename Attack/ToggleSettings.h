/**
    SensIO
    ToggleSettings.h

    Implementation of the "ToggleSettings" - Attack. Transmits data by slightly altering suitable sensor configurations
    (e.g. Thresholds, Reference values, ...). For more information see ***

    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_TOGGLESETTINGS_H
#define SIDECHANNEL_TOGGLESETTINGS_H


#include "AttackBase.h"
#include "../Sensors/SensorBase.h"

/**
 * For now this attack only uses the last two bits of the returned sensor register. Sensor classes could provide
 * additional information about the register, which could improve the attack in terms of flexibility and customization.
 */

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

    const int MAX_RETRYS = 3;
};


#endif //SIDECHANNEL_TOGGLESETTINGS_H

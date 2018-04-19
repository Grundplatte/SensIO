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

    void wait(int cycles) override;

private:
    bool  _last_bit = false;
    int _setting_reg_addr;

    const int MAX_RETRYS = 0;

    const byte_t DATA_MASK = 0x01;
    const byte_t FLAG_MASK = 0x02;
    const byte_t RESET_MASK = 0xFC;

    int write(byte_t &data);
    int read(byte_t &data);
    int waitForAck();
    int ack(byte_t &data);
};


#endif //SIDECHANNEL_TOGGLESETTINGS_H

/**
    SensIO
    AttackBase.h

    Defines classes that attacks need to implement.

    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_ATTACKBASE_H
#define SIDECHANNEL_ATTACKBASE_H

#define TIMEOUT_SHORT 1
#define TIMEOUT_MID 2
#define TIMEOUT_LONG 3
#define TIMEOUT_INFTY 4

#include "../PacketSystem/Packet.h"
#include "../Sensors/SensorBase.h"

class AttackBase {

public:
    /**
     *
     * @param packet
     * @return
     */
    virtual int send(Packet packet) = 0;

    /**
     *
     * @param packet
     * @param scale
     * @return
     */
    virtual int receive(Packet &packet, int scale, bool re_receive) = 0;

    /**
     *
     * @param req
     * @return
     */
    virtual int request(byte_t req) = 0;

    /**
     *
     * @return
     */
    virtual int waitForRequest(byte_t &sqn_had, bool re_receive, bool initial) = 0;


    /**
     *
     * @param cycles
     * @return
     */
    virtual void wait(int cycles) = 0;

protected:
    std::shared_ptr<EDC> _edc;
    std::shared_ptr<SensorBase> _sens;
    std::shared_ptr<spd::logger> _log;
};

#endif //SIDECHANNEL_ATTACKBASE_H

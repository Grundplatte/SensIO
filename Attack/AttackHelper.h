/**
    SensIO
    AttackHelper.h

    Provides helper functions needed for some attacks.

    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_ATTACKHELPER_H
#define SIDECHANNEL_ATTACKHELPER_H


class AttackHelper {
public:
    /**
     * Wait for a specified number of seconds.
     * @param sec Number of seconds to wait.
     * @return
     */
    static int waitS(int sec);

    /**
     * Wait for a specified number of milliseconds.
     * @param ms Number of milliseconds to wait
     * @return
     */
    static int waitMs(int ms);
};


#endif //SIDECHANNEL_ATTACKHELPER_H

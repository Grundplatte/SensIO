/**
    SensIO
    AttackHelper.cpp

    Provides helper functions needed for some attacks.

    @author Markus Feldbacher
*/

#include <time.h>
#include "AttackHelper.h"

int AttackHelper::waitS(int sec) {
    struct timespec req{};
    req.tv_sec = sec;
    req.tv_nsec = 0;

    nanosleep(&req, NULL);
    return 0;
}

int AttackHelper::waitMs(int ms) {
    struct timespec req{};
    req.tv_sec = 0;
    req.tv_nsec = ms * 1000;

    nanosleep(&req, NULL);
    return 0;
}

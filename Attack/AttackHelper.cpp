/**
    SensIO
    AttackHelper.cpp

    Provides helper functions needed for some attacks.

    @author Markus Feldbacher
*/

#include <time.h>
#include "AttackHelper.h"

void AttackHelper::waitS(int sec) {
    struct timespec req{};
    req.tv_sec = sec;
    req.tv_nsec = 0;

    nanosleep(&req, NULL);
}

void AttackHelper::waitMs(long ms) {
    struct timespec req{};
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;

    nanosleep(&req, NULL);
}

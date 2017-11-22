#pragma once

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "../Sensors/HTS221.h"
#include "../Defines.h"
#include "ECC/ECC.h"
#include "EDC/EDC.h"

#define MAXDELAY 200 // ms

class PacketManager {
public:
    PacketManager();
    ~PacketManager();
    int waitForRequest(byte *sqnHad);
    int request(unsigned int sqn);
    int send(byte *packets, unsigned int index);
    int receive(byte *packet);
    int check(byte *packet, unsigned int seq);
    int unpack(byte *packets, unsigned int length, byte **output);
    int create(byte *data, unsigned int length, byte **output);

private:
    int waitForOneTick();
    ECC *m_ECC;
    EDC *m_EDC;
    Sensor *m_sens;
};

//
// Created by Markus Feldbacher on 31.01.18.
//

#ifndef SIDECHANNEL_TESTBED_H
#define SIDECHANNEL_TESTBED_H


#include "PacketSystem/EDC/EDC.h"
#include "PacketSystem/ECC/ECC.h"

class TestBed {
public:
    enum ECCType {

    };

    enum EDCType {

    };

    void setRequestEC(ECCType type);

    void setPacketEC(EDCType type);

private:
    ECC *_requestEC; // TODO: restructure edc/ecc?
    EDC *_packetEC;


};


#endif //SIDECHANNEL_TESTBED_H

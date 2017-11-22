#include <iostream>
#include "PacketSystem/PacketManager.h"
#include "PacketSystem/ECC/Hadamard.h"

typedef uint8_t byte;

int main() {
    printf("[I] Sender started ...\n");


    byte preamble[4] = "pre";
    byte data[21] = "TESTaTESTbTESTcTESTd";

    //detectUsage();
    //send(data, 20   );

    // testing
    //detectUsage();

    byte *packets;
    uint32_t i;
    int numPackets;
    byte sqn, sqnHad, lastseq, cycle;

    int result;
    PacketManager *ps = new PacketManager();
    ECC *ecc = new Hadamard();

    numPackets = ps->create(data, 20, &packets);

    lastseq = 0;
    cycle = 1;
    while(1) {
        // wait for request
        do {
            result = ps->waitForRequest(&sqnHad);
        }while(result < 0);
        printf("seqHad: 0x%x\n", sqnHad);

        result = ecc->decode(&sqnHad, 2, &sqn);

        if(result < 0) {
            //TODO: handle error
            printf("[D] Sequence number (Hadamard) doesnt make sense: %i\n", sqnHad);
            continue;
        }


        if(sqn == lastseq) {
            printf("[D] Receiver is requesting the same packet again: %i\n", sqn);
        }
        else if(sqn != (lastseq+1)%8) {
            printf("[D] Receiver is requesting a packet out of order: %i \n", sqn);
        }

        // seq[0...7]
        if(lastseq > sqn) {
            cycle++;
        }

        ps->send(packets, cycle*8 + sqn);

        lastseq = sqn;
    }

    if(packets != 0)
        free(packets);

    //printf("[D] Sent: 0x%02x 0x%02x 0x%02x 0x%02x = %s\n", data[0], data[1], data[2], data[3], data);

    return(EXIT_SUCCESS);
}
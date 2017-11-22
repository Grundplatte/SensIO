#include <iostream>
#include "PacketSystem/PacketManager.h"

typedef uint8_t byte;

using namespace std;

int main() {
// read until ready bit is toggled off
// waitForSensReady();
// TODO: change to dynamic length
    byte data[20];

    byte packet[3];
    byte timeoutcount;
    int l, x, result;
    PacketManager *ps = new PacketManager();

    for (size_t i = 1; i < 20; i++) {
        timeoutcount = 0;

        ps->request(i);
        result = ps->receive(packet);  // 0=ok, -1=timeout
        cout << "result: " << result << endl;
        while (result < 0) {
            cout << "[I] " << "Timeout, requesting packet" << i << "again" << endl;

            ps->request(i);
            result = ps->receive(packet);  // 0=ok, -1=timeout

        }

        ps->check(packet, i);
    }

    return (EXIT_SUCCESS);
}
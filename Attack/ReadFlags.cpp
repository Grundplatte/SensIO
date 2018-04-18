//
// Created by Markus Feldbacher on 12.04.18.
//

#include "ReadFlags.h"

int ReadFlags::send(Packet packet) {
    return 0;
}

int ReadFlags::receive(Packet &packet, int scale) {
    return 0;
}

int ReadFlags::request(byte_t req) {
    return 0;
}

int ReadFlags::waitForRequest() {
    return 0;
}

int ReadFlags::wait(int cycles) {
    return 0;
}

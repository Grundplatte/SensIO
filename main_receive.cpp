#include <iostream>
#include "spdlog/spdlog.h"
#include "PacketSystem/PacketManager.h"

typedef uint8_t byte;

namespace spd = spdlog;

enum State {
    IDLE,
    REQUEST,
    RECEIVE,
    STOP,
    ERROR
};

int main() {

    std::shared_ptr<spd::logger> log;
    log = spd::stdout_color_mt("main");
    spd::set_pattern("[%M:%S.%e] [%l] [%n] %v");

    /*** SET DEBUG LEVEL ***/
    spd::set_level(spd::level::debug);

    log->info("Receiver started.\n");

    byte data[20];
    byte packet[3];

    int result;
    PacketManager *ps = new PacketManager();

    State state = REQUEST;
    int i = 0;

    while (1) {
        switch (state) {
            case ERROR:
                log->critical("Error -> exiting");
                exit(-1);
                break;

            case REQUEST:
                ps->request(i);

            case RECEIVE:
                result = ps->receive(packet, i);

                // transition
                if (result == 0) {
                    i++;
                    if (i > 2)
                        state = STOP;
                    else
                        state = REQUEST;
                } else if (result == -1)
                    state = RECEIVE;
                else if (result == -2)
                    state = REQUEST;
                else
                    state = ERROR;

                break;

            case STOP:
                log->info("Done. Stopping.");
                ps->request(i - 2);
                exit(0);
                break;

            default:
                break;
        }
    }
}
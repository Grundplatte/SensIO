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

int main(int argc, char *argv[]) {

    std::shared_ptr<spd::logger> log;
    log = spd::stdout_color_mt("main");
    spd::set_pattern("[%M:%S.%e] [%l] [%n] %v");

    /*** SET DEBUG LEVEL ***/
    int c;
    spd::set_level(spd::level::info);
    while ((c = getopt(argc, argv, "dt")) != -1)
        switch (c) {
            case 'd':
                spd::set_level(spd::level::debug);
                break;
            case 't':
                spd::set_level(spd::level::trace);
                break;
            default:
                abort();
        }

    log->info("Receiver started.");
    byte data[21] = "TESTaTESTbTESTcTESTd";

    PacketManager *ps = new PacketManager();
    std::vector<Packet> packets;
    Packet packet;
    int result;

    State state = REQUEST;
    int i = 0;

    ps->printInfo();

    while (1) {
        switch (state) {
            case ERROR:
                log->critical("Error -> exiting");
                exit(-1);
                break;

            case REQUEST:
                ps->request(i);
                state = RECEIVE;
                break;

            case RECEIVE:
                result = ps->receive(packet, i);

                // transition
                if (result == 0) {
                    packets.push_back(packet);
                    i++;
                    switch (packet.getCommand()) {
                        case Packet::CMD_UP:
                            state = REQUEST;
                            ps->scaleUp();
                            break;
                        case Packet::CMD_DOWN:
                            state = REQUEST;
                            ps->scaleDown();
                            break;
                        case Packet::CMD_STOP:
                            state = STOP;
                            break;
                        default:
                            state = REQUEST;
                    }
                } else if (result == -1) {
                    state = RECEIVE;
                    ps->wait(1);
                } else if (result == -2) {
                    state = REQUEST;
                }
                else
                    state = ERROR;

                break;

            case STOP:
                log->info("Done. Stopping.");
                ps->request(i - 2);

                byte *output;
                ps->unpack(packets, &output);
                log->info("Unpacked: {}", output);
                free(output);

                exit(0);
                break;

            default:
                break;
        }
    }
}
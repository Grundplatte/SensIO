#include <iostream>
#include "spdlog/spdlog.h"
#include "PacketSystem/PacketManager.h"

typedef uint8_t byte;

namespace spd = spdlog;

enum State {
    IDLE,
    REQUEST,
    RECEIVE,
    RERECEIVE,
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

    PacketManager *manager = new PacketManager();
    std::vector<Packet> packets;
    Packet packet;
    int result;
    int scale = 2;

    State state = REQUEST;
    int i = 0;

    manager->printInfo();

    while (true) {
        switch (state) {
            case ERROR:
                log->critical("Error -> exiting");
                return -1;

            case REQUEST:
                manager->request(i);
                state = RECEIVE;
                break;

            case RECEIVE:
                result = manager->receive(packet, i, scale, 0);

                // transition
                if (result == 0) {
                    i++;
                    manager->wait(1);
                    if (packet.isCommand()) {
                        switch (packet.getCommand()) {
                            case Packet::CMD_UP:
                                state = REQUEST;
                                scale++;
                                log->info("Scaling up to: {}", scale);
                                break;
                            case Packet::CMD_DOWN:
                                state = REQUEST;
                                scale--;
                                log->info("Scaling down to: {}", scale);
                                break;
                            case Packet::CMD_STOP:
                                state = STOP;
                                break;
                            default:
                                state = REQUEST;
                        }
                    } else {
                        packets.push_back(packet);
                        state = REQUEST;
                    }
                } else if (result == -1) {
                    state = RERECEIVE;
                    //manager->wait(1);
                } else if (result == -2) {
                    state = REQUEST;
                    //manager->wait(1);
                }
                else
                    state = ERROR;

                break;

            case RERECEIVE:
                result = manager->receive(packet, i, scale, 1);

                // transition
                if (result == 0) {
                    i++;
                    if (packet.isCommand()) {
                        switch (packet.getCommand()) {
                            case Packet::CMD_UP:
                                state = REQUEST;
                                scale++;
                                log->info("Scaling up to: {}", scale);
                                break;
                            case Packet::CMD_DOWN:
                                state = REQUEST;
                                scale--;
                                log->info("Scaling down to: {}", scale);
                                break;
                            case Packet::CMD_STOP:
                                state = STOP;
                                break;
                            default:
                                state = REQUEST;
                        }
                    } else {
                        packets.push_back(packet);
                        state = REQUEST;
                    }
                } else if (result == -1) {
                    state = RERECEIVE;
                    //manager->wait(1);
                } else if (result == -2) {
                    state = REQUEST;
                    //manager->wait(1);
                } else
                    state = ERROR;

                break;

            case STOP:
                log->info("Done. Stopping.");
                manager->request(i - 2);

                byte *output;
                manager->unpack(packets, &output);
                log->info("Unpacked: {}", output);
                free(output);

                return 0;

            case IDLE:
                break;
        }
    }
}
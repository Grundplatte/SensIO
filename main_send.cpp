#include <iostream>
#include <bitset>
#include "spdlog/spdlog.h"
#include "PacketSystem/PacketManager.h"
#include "PacketSystem/ECC/Hadamard.h"

namespace spd = spdlog;

enum State {
    WAIT_FOR_SQN,
    CHECK_FOR_SQN,
    DECODE_SQN,
    SEND_PACKET,
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

    log->info("Sender started.");

    byte data[21] = "TESTaTESTbTESTcTESTd";
    Packet nextPacket;
    byte sqnHad;
    int result;
    int error_count = 0;
    int success_count = 0;

    PacketManager ps = PacketManager();
    PacketFactory factory = PacketFactory(data, 21);

    ECC *ecc = new Hadamard();
    State state = WAIT_FOR_SQN;

    int packetSqn = -1;
    int modSqn = MAX_SQN + 1;

    //ps.printInfo();
    //ps.pack(data, 20, packets);


    while(1) {
        switch (state) {
            case ERROR:
                log->critical("Error -> exiting.\n");
                exit(-1);
                break;

            case WAIT_FOR_SQN:
                result = ps.waitForRequest(&sqnHad);

                if (result == 0)
                    state = DECODE_SQN;

                break;

            case CHECK_FOR_SQN:
                result = ps.checkForRequest(&sqnHad);

                if (result == -2) {
                    state = SEND_PACKET;
                    ps.wait(1); // TODO: ??
                }
                else if (result == 0)
                    state = DECODE_SQN;

                break;

            case DECODE_SQN:
                result = ps.checkRequest(&sqnHad);
                //result = ecc->check(&sqnHad, 7);
                //ecc->decode(&sqnHad, 8, &sqn);

                if (result < 0) {
                    log->warn("SQN not valid, check again.");
                    state = CHECK_FOR_SQN;
                } else if (result == (packetSqn - 1) % modSqn) // stop condition
                    state = STOP;
                else if (result == (packetSqn) % modSqn) {
                    log->warn("Receiver requested the same packet again.");
                    if (!nextPacket.isCommand()) {
                        // Packet may be too big, try scaling down
                        error_count++;
                        success_count = 0;
                        if (factory.scaleDown() == 0) {
                            factory.previous();
                            factory.getCommandPacket(Packet::CMD_DOWN, packetSqn, nextPacket);
                            log->info("Packet size reduced");
                        }
                    }
                    state = SEND_PACKET;
                } else if (result == (packetSqn + 1) % modSqn) {

                    result = factory.getNextPacket(++packetSqn, nextPacket);
                    if (result == -1) {
                        // no packets left, send stop
                        factory.getCommandPacket(Packet::CMD_STOP, packetSqn, nextPacket);
                    }
                    /*
                    if(success_count++ == P_TEST_UPSCALE){
                        success_count = 0;
                        if(factory.scaleUp() == 0){
                            factory.getCommandPacket(Packet::CMD_UP, packetSqn, nextPacket);
                            log->info("Packet size increased");
                        }
                    }*/
                    error_count = 0;

                    state = SEND_PACKET;
                } else {
                    log->error("Receiver requested packet out of order!");
                    state = STOP;
                }

                break;

            case SEND_PACKET:
                log->info("Sending packet {0}", packetSqn);
                //ps.send(packets.at(packetSqn));
                ps.send(nextPacket);
                state = CHECK_FOR_SQN;
                break;

            case STOP:
                log->info("Stopping.");
                return (0);
                break;
        }
    }
}
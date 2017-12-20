#include <iostream>
#include <bitset>
#include "spdlog/spdlog.h"
#include "PacketSystem/PacketManager.h"
#include "PacketSystem/ECC/Hadamard.h"
#include "PacketSystem/EDC/Berger.h"

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

    log->info("Sender started. Waiting for Request");

    byte data[21] = "TESTaTESTbTESTcTESTd";
    std::vector<std::vector<bit_t> > packets;
    byte sqn, sqnHad;
    int result;

    PacketManager *ps = new PacketManager();
    ECC *ecc = new Hadamard();
    State state = WAIT_FOR_SQN;

    int packetSqn = -1;
    int modSqn = ps->getMaxSqn();

    ps->printInfo();
    ps->pack(data, 20, packets);

    while(1) {
        switch (state) {
            case ERROR:
                log->critical("Error -> exiting.\n");
                exit(-1);
                break;

            case WAIT_FOR_SQN:
                result = ps->waitForRequest(&sqnHad);

                if (result == 0)
                    state = DECODE_SQN;

                break;

            case CHECK_FOR_SQN:
                result = ps->checkForRequest(&sqnHad);

                if (result == -2) {
                    state = SEND_PACKET;
                    ps->wait(1);
                }
                else if (result == 0)
                    state = DECODE_SQN;

                break;

            case DECODE_SQN:
                result = ecc->check(&sqnHad, 7);
                ecc->decode(&sqnHad, 8, &sqn);

                if (result < 0) {
                    log->warn("SQN not valid, check again.");
                    state = CHECK_FOR_SQN;
                }
                else if (sqn == (packetSqn - 1) % modSqn) // stop condition
                    state = STOP;
                else if (sqn == (packetSqn) % modSqn) {
                    log->warn("Receiver requested the same packet again.");
                    state = SEND_PACKET;
                } else if (sqn == (packetSqn + 1) % modSqn) {
                    packetSqn++;
                    state = SEND_PACKET;
                } else {
                    log->error("Receiver requested packet out of order!");
                    state = STOP;
                }

                // TODO: check sqn flow (newSqn = oldSqn+1)
                // TODO: calculate real sqn (++)
                // TODO: transmit next on +1, same sqn shouldnt happen but retransmit,
                // ignore other sqn a few times, then error/restart depending on sqn

                break;

            case SEND_PACKET:
                log->info("Sending packet {0}", packetSqn);
                ps->send(packets.at(packetSqn));
                state = CHECK_FOR_SQN;
                break;

            case STOP:
                log->info("Stopping.");
                return (0);
                break;
        }
    }
}
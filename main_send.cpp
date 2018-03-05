#include <iostream>
#include <bitset>
#include "spdlog/spdlog.h"
#include "PacketSystem/PacketManager.h"
#include "TestBed.h"
#include <fstream>
#include <sys/mman.h>

namespace spd = spdlog;

enum State {
    WAIT_FOR_SQN,
    CHECK_FOR_SQN,
    RECHECK_FOR_SQN,
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

    FILE *file = fopen("simple.bmp", "rb");
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *filecontent = (unsigned char *) mmap(0, filesize, PROT_READ, MAP_PRIVATE, fileno(file), 0);
    if (!filecontent)
        exit(EXIT_FAILURE);

    TestBed testBed = TestBed();
    testBed.setHAL(TestBed::HAL_I2C);
    testBed.setSensor(TestBed::SENSOR_HTS221);
    testBed.setRequestECC(TestBed::ECC_HADAMARD);
    testBed.setPacketEDC(TestBed::EDC_BERGER);
    testBed.setTestBuffer(filecontent, filesize);

    testBed.runTest(true);

    fclose(file);
}
/**
    SensIO
    main_send.cpp

    Main function for the sender.

    @author Markus Feldbacher
*/

#include <iostream>
#include <bitset>
#include "spdlog/spdlog.h"
#include "PacketSystem/PacketManager.h"
#include "TestBed.h"
#include "Sensors/HTS221.h"
#include "Sensors/HAL/I2C_HAL.h"
#include <fstream>
#include <sys/mman.h>

namespace spd = spdlog;

int main(int argc, char *argv[]) {
    std::shared_ptr<spd::logger> log;
    log = spd::stdout_color_mt("main");
    spd::set_pattern("[%M:%S.%e] [%l] [%n] %v");
    char *filename;

    /*** SET DEBUG LEVEL ***/
    int c;
    spd::set_level(spd::level::info);
    while ((c = getopt(argc, argv, "dti:")) != -1)
        switch (c) {
            case 'd':
                spd::set_level(spd::level::debug);
                break;
            case 't':
                spd::set_level(spd::level::trace);
                break;
            case 'i':
                filename = optarg;
                break;
            default:
                abort();
        }

    log->info("Sender started.");

    FILE *file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *filecontent = (unsigned char *) mmap(0, filesize, PROT_READ, MAP_PRIVATE, fileno(file), 0);
    if (!filecontent)
        exit(EXIT_FAILURE);

    TestBed testBed = TestBed();
    testBed.setRequestECC(TestBed::ECC_HADAMARD);
    testBed.setPacketEDC(TestBed::EDC_BERGER);
    testBed.setHAL(TestBed::HAL_I2C);
    testBed.setSensor(TestBed::SENSOR_HTS221);
    testBed.setAttack(TestBed::ATTACK_READFLAGS);
    testBed.setTestBuffer(filecontent, filesize);

    testBed.runTest(true);

    fclose(file);
}
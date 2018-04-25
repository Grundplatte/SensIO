/**
    SensIO
    main_receive.cpp

    Main function for the receiver.

    @author Markus Feldbacher
*/

#include <iostream>
#include "spdlog/spdlog.h"
#include "PacketSystem/PacketManager.h"
#include "TestBed.h"
#include "Sensors/HTS221.h"
#include "PacketSystem/EDC/Berger.h"
#include "Sensors/HAL/I2C_HAL.h"

namespace spd = spdlog;

int main(int argc, char *argv[]) {

    std::shared_ptr<spd::logger> log;
    log = spd::stdout_color_mt("main");
    spd::set_pattern("[%M:%S.%e] [%l] [%n] %v");
    char *outfilename, *infilename;

    /*** SET DEBUG LEVEL ***/
    int c;
    spd::set_level(spd::level::info);
    while ((c = getopt(argc, argv, "dti:o:")) != -1)
        switch (c) {
            case 'd':
                spd::set_level(spd::level::debug);
                break;
            case 't':
                spd::set_level(spd::level::trace);
                break;
            case 'i':
                infilename = optarg;
                break;
            case 'o':
                outfilename = optarg;
                break;
            default:
                abort();
        }

    log->info("Receiver started.");

    // TODO: use dynamic buffer
    FILE *file_temp = fopen(infilename, "rb");
    fseek(file_temp, 0, SEEK_END);
    long filesize = ftell(file_temp);
    fclose(file_temp);

    unsigned char *buf = (unsigned char *) malloc(filesize);
    if (!buf)
        exit(EXIT_FAILURE);

    log->info("Receiver started.");

    TestBed testBed = TestBed();
    testBed.setRequestECC(TestBed::ECC_HADAMARD);
    testBed.setPacketEDC(TestBed::EDC_BERGER);
    testBed.setHAL(TestBed::HAL_I2C);
    testBed.setSensor(TestBed::SENSOR_HTS221);
    testBed.setAttack(TestBed::ATTACK_READFLAGS);
    testBed.setTestBuffer(buf, filesize);

    testBed.runTest(false);

    FILE *file = fopen(outfilename, "wb");
    fwrite(buf, sizeof(unsigned char), filesize, file);
    fclose(file);

    free(buf);
}
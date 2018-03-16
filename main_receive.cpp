#include <iostream>
#include "spdlog/spdlog.h"
#include "PacketSystem/PacketManager.h"
#include "TestBed.h"

typedef uint8_t byte_t;

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
    testBed.setHAL(TestBed::HAL_I2C);
    testBed.setSensor(TestBed::SENSOR_LPS25H_UNUSED);
    testBed.setRequestECC(TestBed::ECC_HADAMARD);
    testBed.setPacketEDC(TestBed::EDC_NOEDC);
    testBed.setTestBuffer(buf, filesize);

    testBed.runTest(false);

    FILE *file = fopen(outfilename, "wb");
    fwrite(buf, sizeof(unsigned char), filesize, file);
    fclose(file);

    free(buf);
}
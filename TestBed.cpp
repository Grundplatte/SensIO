//
// Created by Markus Feldbacher on 31.01.18.
//

#include "TestBed.h"
#include "PacketSystem/ECC/NoECC.h"
#include "PacketSystem/EDC/NoEDC.h"
#include "PacketSystem/ECC/Hadamard.h"
#include "PacketSystem/EDC/Berger.h"
#include "Sensors/HAL/SPI_HAL.h"
#include "Sensors/HAL/I2C_HAL.h"
#include "Sensors/HTS221Flags.h"
#include "Sensors/LPS25HUnused.h"

namespace spd = spdlog;

void TestBed::setRequestECC(int type) {
    switch (type) {
        case ECC_NOECC:
            _requestECC = std::shared_ptr<ECC>(new NoECC());
            _log->trace("Request ecc set to: NoECC");
            break;
        case ECC_HADAMARD:
            _requestECC = std::shared_ptr<ECC>(new Hadamard());
            _log->trace("Request ecc set to: Hadamard");
            break;
        default:
            _requestECC = std::shared_ptr<ECC>(new Hadamard());
            _log->trace("Request ecc set to: default (Hadamard)");
    }
}

void TestBed::setPacketEDC(int type) {
    switch (type) {
        case EDC_NOEDC:
            _packetEDC = std::shared_ptr<EDC>(new NoEDC());
            _log->trace("Packet edc set to: NoEDC");
            break;
        case EDC_BERGER:
            _packetEDC = std::shared_ptr<EDC>(new Berger());
            _log->trace("Packet edc set to: Berger");
            break;
        default:
            _packetEDC = std::shared_ptr<EDC>(new Berger());
            _log->trace("Packet edc set to: default (Berger)");
    }
}

void TestBed::setHAL(int halType) {
    switch (halType) {
        case HAL_I2C:
            _hal = std::shared_ptr<HAL>(new I2C_HAL());
            _log->trace("HAL set to: I2C");
            break;
        case HAL_SPI:
            _hal = std::shared_ptr<HAL>(new SPI_HAL());
            _log->trace("HAL set to: SPI");
            break;
        default:
            _log->error("Unknown HAL type!");
    }
}

void TestBed::setSensor(int sensorType) {
    if (!_hal) {
        _log->error("Please set HAL before sensor!");
        return;
    }

    switch (sensorType) {
        case SENSOR_HTS221_FLAGS:
            // FIXME: why cast?
            _sensor = std::shared_ptr<Sensor>(new HTS221Flags(_hal));
            _log->trace("Sensor set to: HTS221Flags");
            break;
        case SENSOR_LPS25H_UNUSED:
            _sensor = std::shared_ptr<Sensor>(new LPS25HUnused(_hal));
            _log->trace("Sensor set to: LPS25Unused");
            break;
        default:
            _log->error("Unknown sensor/attack type!");
    }
}

int TestBed::runTest(bool send) {

    // checks
    if (!_packetEDC) {
        _log->error("No EDC set!");
        return -1;
    }

    if (!_requestECC) {
        _log->error("No ECC set!");
        return -1;
    }

    if (!_hal) {
        _log->error("No HAL set!");
        return -1;
    }

    if (!_sensor) {
        _log->error("No sensor set!");
        return -1;
    }


    if (send && _buf_len == 0) {
        _log->error("Send mode, but no data!");
        return -1;
    }
    _pm = std::unique_ptr<PacketManager>(new PacketManager(_requestECC, _packetEDC, _sensor));
    _pf = std::unique_ptr<PacketFactory>(new PacketFactory());


    // run tests
    if (send)
        return runTestSend();
    else
        return runTestReceive();
}

TestBed::TestBed() {
    // <-- Log settings -->
    std::shared_ptr<spdlog::logger> log = spd::get("PMgr");
    _log = log ? log : spd::stdout_color_mt("PMgr");
}

int TestBed::runTestSend() {
    _log->trace("runTestSend");

    Packet next_packet;
    byte_t sqn_had;
    int result;
    int error_count = 0;
    int success_count = 0;

    StateS state = S_WAIT_FOR_SQN;

    int packetSqn = -1;
    int modSqn = MAX_SQN + 1;

    // setup data
    _log->trace("Setting up databuffer...");
    _pf->appendData(_buf, _buf_len);

    while (true) {
        switch (state) {
            case S_ERROR:
                _log->critical("Error -> exiting.\n");
                return -1;

            case S_WAIT_FOR_SQN:
                result = _pm->waitForRequest(&sqn_had);

                if (result == 0)
                    state = S_DECODE_SQN;

                break;

            case S_CHECK_FOR_SQN:
                result = _pm->checkForRequest(&sqn_had, 0);

                if (result == -2) {
                    error_count++;
                    state = S_SEND_PACKET;
                    //manager.wait(1); // TODO: ??
                } else if (result == -1) {
                    state = S_RECHECK_FOR_SQN;
                } else if (result == 0)
                    state = S_DECODE_SQN;

                break;

            case S_RECHECK_FOR_SQN:
                result = _pm->checkForRequest(&sqn_had, 1);

                if (result == -2) {
                    state = S_SEND_PACKET;
                    //manager.wait(1); // TODO: ??
                } else if (result == -1) {
                    state = S_RECHECK_FOR_SQN;
                } else if (result == 0)
                    state = S_DECODE_SQN;

                break;

            case S_DECODE_SQN:
                result = _pm->checkRequest(&sqn_had);
                //result = ecc->check(&sqnHad, 7);
                //ecc->decode(&sqnHad, 8, &sqn);

                if (result < 0) {
                    _log->warn("SQN not valid, check again.");
                    state = S_RECHECK_FOR_SQN;
                } else if (result == (packetSqn - 1) % modSqn) // stop condition
                    state = S_STOP;
                else if (result == (packetSqn) % modSqn) {
                    _log->warn("Receiver requested the same packet again.");
                    //manager.wait(1);
                    state = S_SEND_PACKET;
                } else if (result == (packetSqn + 1) % modSqn) {

                    result = _pf->getNextPacket(++packetSqn, next_packet);
                    if (result == -1) {
                        // no packets left, send stop
                        _pf->getCommandPacket(Packet::CMD_STOP, packetSqn, next_packet);
                        break;
                    }

                    if (success_count++ == P_TEST_UPSCALE) {
                        success_count = 0;
                        if (_pf->scaleUp() == 0) {
                            _pf->previous();
                            _pf->getCommandPacket(Packet::CMD_UP, packetSqn, next_packet);
                            _log->info("Packet size increased");
                        }
                    }
                    error_count = 0;

                    //_pm->wait(1);
                    state = S_SEND_PACKET;
                } else {
                    _log->error("Receiver requested packet out of order!");
                    state = S_STOP;
                }

                break;

            case S_SEND_PACKET:
                _log->info("Sending packet {0}", packetSqn);
                if (error_count > 0 && !next_packet.isCommand()) {
                    // Packet may be too big, try scaling down
                    error_count = 0;
                    success_count = 0;
                    if (_pf->scaleDown() == 0) {
                        _pf->previous();
                        _pf->getCommandPacket(Packet::CMD_DOWN, packetSqn, next_packet);
                        _log->info("Packet size reduced");
                    }
                }
                _pm->send(next_packet);
                state = S_CHECK_FOR_SQN;
                break;

            case S_STOP:
                _log->info("Stopping.");
                return 0;
        }
    }
    return 0;
}

int TestBed::runTestReceive() {
    std::vector<Packet> packets;
    Packet packet;
    int result;
    int scale = 2;

    StateR state = R_REQUEST;
    int i = 0;

    while (true) {
        switch (state) {
            case R_ERROR:
                _log->critical("Error -> exiting");
                return -1;

            case R_REQUEST:
                _pm->request(i);
                state = R_RECEIVE;
                break;

            case R_RECEIVE:
                result = _pm->receive(packet, i, scale, 0);

                // transition
                if (result == 0) {
                    i++;
                    //_pm->wait(1);
                    if (packet.isCommand()) {
                        switch (packet.getCommand()) {
                            case Packet::CMD_UP:
                                state = R_REQUEST;
                                scale++;
                                _log->info("Scaling up to: {}", scale);
                                break;
                            case Packet::CMD_DOWN:
                                state = R_REQUEST;
                                scale--;
                                _log->info("Scaling down to: {}", scale);
                                break;
                            case Packet::CMD_STOP:
                                state = R_STOP;
                                break;
                            default:
                                state = R_REQUEST;
                        }
                    } else {
                        packets.push_back(packet);
                        state = R_REQUEST;
                    }
                } else if (result == -1) {
                    state = R_RERECEIVE;
                    //manager->wait(1);
                } else if (result == -2) {
                    state = R_REQUEST;
                    //manager->wait(1);
                } else
                    state = R_ERROR;

                break;

            case R_RERECEIVE:
                result = _pm->receive(packet, i, scale, 1);

                // transition
                if (result == 0) {
                    i++;
                    if (packet.isCommand()) {
                        switch (packet.getCommand()) {
                            case Packet::CMD_UP:
                                state = R_REQUEST;
                                scale++;
                                _log->info("Scaling up to: {}", scale);
                                break;
                            case Packet::CMD_DOWN:
                                state = R_REQUEST;
                                scale--;
                                _log->info("Scaling down to: {}", scale);
                                break;
                            case Packet::CMD_STOP:
                                state = R_STOP;
                                break;
                            default:
                                state = R_REQUEST;
                        }
                    } else {
                        packets.push_back(packet);
                        state = R_REQUEST;
                    }
                } else if (result == -1) {
                    state = R_RERECEIVE;
                    //manager->wait(1);
                } else if (result == -2) {
                    state = R_REQUEST;
                    //manager->wait(1);
                } else
                    state = R_ERROR;

                break;

            case R_STOP:
                _log->info("Done. Stopping.");
                _pm->request(i - 2);

                _pm->unpack(packets, _buf, _buf_len);
                //_log->info("Unpacked: {}", output);
                //free(output);

                return 0;

            case R_IDLE:
                break;
        }
    }
    return 0;
}

void TestBed::setTestBuffer(unsigned char *data, int length) {
    _buf = data;
    _buf_len = length;
}


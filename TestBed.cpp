/**
    SensIO
    TestBed.cpp

    Implementation of the testbed, that handles the selection of different aspects of the attack
    (e.g. channel/attack type, hal, sensor, ...).

    @author Markus Feldbacher
*/

#include "TestBed.h"
#include "PacketSystem/EDC/NoEDC.h"
#include "PacketSystem/ECC/Hadamard.h"
#include "PacketSystem/EDC/Berger.h"
#include "Sensors/HAL/SPI_HAL.h"
#include "Sensors/HAL/I2C_HAL.h"
#include "Sensors/HTS221.h"
#include "Sensors/LPS25H.h"
#include "Attack/UnusedRegisters.h"
#include "Attack/ToggleSettings.h"
#include "Attack/ReadFlags.h"

namespace spd = spdlog;

bool TestBed::TYPE = -1; // dummy

void TestBed::setRequestECC(int type) {
    switch (type) {
        case ECC_HADAMARD:
            _requestECC = std::shared_ptr<ECC>(new Hadamard());
            _log->debug("Request ecc set to: Hadamard");
            break;
        default:
            _requestECC = std::shared_ptr<ECC>(new Hadamard());
            _log->debug("Request ecc set to: default (Hadamard)");
    }
}

void TestBed::setPacketEDC(int type) {
    switch (type) {
        case EDC_NOEDC:
            _packetEDC = std::shared_ptr<EDC>(new NoEDC());
            _log->debug("Packet edc set to: NoEDC");
            break;
        case EDC_BERGER:
            _packetEDC = std::shared_ptr<EDC>(new Berger());
            _log->debug("Packet edc set to: Berger");
            break;
        default:
            _packetEDC = std::shared_ptr<EDC>(new Berger());
            _log->debug("Packet edc set to: default (Berger)");
    }
}

void TestBed::setHAL(int halType) {
    switch (halType) {
        case HAL_I2C:
            _hal = std::shared_ptr<HAL>(new I2C_HAL());
            _log->debug("HAL set to: I2C");
            break;
        case HAL_SPI:
            _hal = std::shared_ptr<HAL>(new SPI_HAL());
            _log->debug("HAL set to: SPI");
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
        case SENSOR_LPS25H:
            _sensor = std::shared_ptr<SensorBase>(new LPS25H(_hal));
            _log->debug("Sensor set to: LPS25H");
            break;
        case SENSOR_HTS221:
            _sensor = std::shared_ptr<SensorBase>(new HTS221(_hal));
            _log->debug("Sensor set to: HTS221");
            break;
        default:
            _log->error("Unknown sensor.");
    }
}

void TestBed::setAttack(int attackType) {
    if(!_sensor) {
        _log->error("Please set sensor before attack!");
        return;
    }

    switch (attackType) {
        case ATTACK_READFLAGS:
            _attack = std::shared_ptr<AttackBase>(new ReadFlags(_packetEDC, _sensor));
            _log->debug("Attack set to: Read Flags");
            break;
        case ATTACK_TOGGLESET:
            _attack = std::shared_ptr<AttackBase>(new ToggleSettings(_packetEDC, _sensor));
            _log->debug("Attack set to: Toggle Settings");
            break;
        case ATTACK_UNUSEDREG:
            _attack = std::shared_ptr<AttackBase>(new UnusedRegisters(_packetEDC, _sensor));
            _log->debug("Attack set to: Unused Registers");
            break;
        default:
            _log->error("Unknown attack.");
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
    _pm = std::unique_ptr<PacketManager>(new PacketManager(_requestECC, _packetEDC, _attack));
    _pf = std::unique_ptr<PacketFactory>(new PacketFactory(_packetEDC));


    TestBed::TYPE = send;

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

    Packet next_packet(_packetEDC);
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


    Packet p = Packet(0,0,_packetEDC);
    while(1) {
        _pm->send(p);
    }

    while (true) {
        switch (state) {
            case S_ERROR:
                _log->critical("Error -> exiting.\n");
                return -1;

            case S_WAIT_FOR_SQN:
                result = _pm->waitForRequest(sqn_had);

                if (result == STATUS_OK){
                    state = S_DECODE_SQN;
                    clock_gettime(CLOCK_REALTIME, &_start_time); //FIXME: does not work if user is present before receiver
                }

                break;

            case S_CHECK_FOR_SQN:
                result = _pm->checkForRequest(sqn_had, 0);

                if (result == TIMEOUT_WHILE_WAITING) {
                    error_count++;
                    success_count = 0;
                    _retrans_count++;
                    state = S_SEND_PACKET;
                    //manager.wait(1);
                } else if (result == TIMEOUT_WHILE_RECEIVING) {
                    _packet_count++;
                    _retrans_count++;
                    state = S_RECHECK_FOR_SQN;
                } else if (result == STATUS_OK){
                    _packet_count++;
                    state = S_DECODE_SQN;
                }

                break;

            case S_RECHECK_FOR_SQN:
                result = _pm->checkForRequest(sqn_had, 1);

                if (result == -2) {
                    state = S_SEND_PACKET;
                } else if (result == -1) {
                    state = S_RECHECK_FOR_SQN;
                } else if (result == 0)
                    state = S_DECODE_SQN;

                break;

            case S_DECODE_SQN:
                result = _pm->validateRequest(sqn_had);

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
                        state = S_SEND_PACKET;
                        break;
                    }

                    error_count = 0;
                    if (success_count++ == 2) {
                        success_count = 0;
                        if (_pf->scaleUp() == 0) {
                            _pf->previous();
                            _pf->getCommandPacket(Packet::CMD_UP, packetSqn, next_packet);
                            _log->info("Packet size increased");
                        }
                    }

                    _pm->wait(1);
                    state = S_SEND_PACKET;
                } else {
                    _log->error("Receiver requested packet out of order!");
                    state = S_STOP;
                }

                break;

            case S_SEND_PACKET:
                _log->info("Sending packet {0}", packetSqn);
                if (error_count > 1 && !next_packet.isCommand()) {
                    // Packet may be too big, try scaling down
                    error_count = 0;
                    success_count = 0;
                    if (_pf->scaleDown() == 0) {
                        _upscale_factor *= 2;
                        _pf->previous();
                        _pf->getCommandPacket(Packet::CMD_DOWN, packetSqn, next_packet);
                        _log->info("Packet size reduced");
                    }
                }
                _pm->send(next_packet);
                _packet_count++;
                state = S_CHECK_FOR_SQN;
                break;

            case S_STOP:
                _log->info("Stopping.");
                clock_gettime(CLOCK_REALTIME, &_end_time);
                printStats();
                return 0;
        }
    }
    return 0;
}

int TestBed::runTestReceive() {
    std::vector<Packet> packets;
    Packet packet(_packetEDC);
    int result;
    int scale = P_INIT_SCALE;

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
                if (result == STATUS_OK) {
                    i++;
                    _pm->wait(1);
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
                } else if (result == TIMEOUT_WHILE_RECEIVING) {
                    state = R_RERECEIVE;
                    //manager->wait(1);
                } else if (result == TIMEOUT_WHILE_WAITING) {
                    state = R_REQUEST;
                    //manager->wait(1);
                } else
                    state = R_ERROR;

                break;

            case R_RERECEIVE:
                result = _pm->receive(packet, i, scale, 1);

                // transition
                if (result == STATUS_OK) {
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
                } else if (result == TIMEOUT_WHILE_RECEIVING) {
                    state = R_RERECEIVE;
                    //manager->wait(1);
                } else if (result == TIMEOUT_WHILE_WAITING) {
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

void TestBed::printStats() {
    _log->info("======= Statistics =======");
    _log->info("= #Packets: {0:2}", _packet_count);
    _log->info("= #Retransmissions: {0}", _retrans_count);
    _log->info("= Time: {0}:{1}", (_end_time.tv_sec - _start_time.tv_sec)/60, (_end_time.tv_sec - _start_time.tv_sec)%60);
    _log->info("==========================");
}


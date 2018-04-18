# cmake_minimum_required(VERSION <specify CMake version here>)
project(Sidechannel)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)

include_directories(wiringPi)
link_directories(wiringPi)


# general stuff
set(SOURCE_FILES_LIB PacketSystem/ECC/Viterbi.cpp Sensors/HTS221Flags.cpp Sensors/LPS25HUnused.cpp Sensors/LPS25HSettings.cpp
        Sensors/HAL/I2C_HAL.cpp PacketSystem/PacketManager.cpp PacketSystem/ECC/Hadamard.cpp PacketSystem/EDC/Berger.cpp
        PacketSystem/EDC/NoEDC.cpp PacketSystem/Packet.cpp PacketSystem/Packet.h PacketSystem/PacketFactory.cpp
        PacketSystem/PacketFactory.h Sensors/HAL/SPI_HAL.cpp Sensors/HAL/SPI_HAL.h TestBed.cpp TestBed.h
        ChannelTypes/ReadFlags.cpp ChannelTypes/ReadFlags.h Sensors/LPS25H.cpp Sensors/LPS25H.h Sensors/SensorBase.h Attack/UnusedRegisters.cpp Attack/UnusedRegisters.h Attack/ToggleSettings.cpp Attack/ToggleSettings.h Attack/ReadFlags.cpp Attack/ReadFlags.h Sensors/HTS221.cpp Sensors/HTS221.h)
set(HEADER_FILES_LIB PacketSystem/ECC/ECC.h PacketSystem/EDC/EDC.h Sensors/Sensor.h Sensors/HAL/HAL.h Attack/AttackBase.h)

set(SOURCE_FILES_SENDER ${SOURCE_FILES_LIB} main_send.cpp)
set(SOURCE_FILES_RECEIVER ${SOURCE_FILES_LIB} main_receive.cpp)

# executables
add_executable(send ${SOURCE_FILES_SENDER} ${HEADER_FILES_LIB})
add_executable(receive ${SOURCE_FILES_RECEIVER} ${HEADER_FILES_LIB})

target_link_libraries(send pthread)
target_link_libraries(receive pthread)
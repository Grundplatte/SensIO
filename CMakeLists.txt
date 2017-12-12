# cmake_minimum_required(VERSION <specify CMake version here>)
project(Sidechannel)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)

# general stuff
set(SOURCE_FILES_LIB PacketSystem/ECC/Viterbi.cpp Sensors/HTS221.cpp Sensors/HAL/I2C_HAL.cpp PacketSystem/PacketManager.cpp PacketSystem/ECC/Hadamard.cpp PacketSystem/EDC/Berger.cpp PacketSystem/ECC/NoECC.cpp PacketSystem/EDC/NoEDC.cpp)


set(SOURCE_FILES_SENDER ${SOURCE_FILES_LIB} main_send.cpp)
set(SOURCE_FILES_RECEIVER ${SOURCE_FILES_LIB} main_receive.cpp)

# executables
add_executable(send ${SOURCE_FILES_SENDER})
add_executable(receive ${SOURCE_FILES_RECEIVER})

target_link_libraries(send pthread)
target_link_libraries(receive pthread)
cmake_minimum_required(VERSION 3.10)

project(Sidechannel)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)

include_directories(wiringPi/wiringPi)
link_directories(wiringPi/wiringPi)


# general stuff
file(GLOB_RECURSE SOURCE_FILES_LIB "Attack/*.cpp" "PacketSystem/*.cpp" "Sensors/*.cpp")
file(GLOB_RECURSE HEADER_FILES_LIB "Attack/*.h" "PacketSystem/*.h" "Sensors/*.h")

set(SOURCE_FILES_LIB ${SOURCE_FILES_LIB} TestBed.cpp)
set(HEADER_FILES_LIB ${HEADER_FILES_LIB} TestBed.h Defines.h)

set(SOURCE_FILES_SENDER ${SOURCE_FILES_LIB} main_send.cpp)
set(SOURCE_FILES_RECEIVER ${SOURCE_FILES_LIB} main_receive.cpp)

# executables
add_executable(send ${SOURCE_FILES_SENDER} ${HEADER_FILES_LIB})
add_executable(receive ${SOURCE_FILES_RECEIVER} ${HEADER_FILES_LIB})

target_link_libraries(send pthread)
target_link_libraries(receive pthread)
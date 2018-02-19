//
// Created by Markus Feldbacher on 30.01.18.
//

#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>
#include "SPI_HAL.h"

int SPI_HAL::read(byte slave_addr, byte regAddr, unsigned int length, byte *data) {
    return 0;
}

int SPI_HAL::write(byte slave_addr, byte reg_addr, unsigned int length, byte *data) {
    return 0;
}

SPI_HAL::SPI_HAL() {
    _spi = open(SPI_INTERFACE, O_RDWR);
    if (_spi < 0) {
        exit(EXIT_FAILURE);
    }
}

SPI_HAL::~SPI_HAL() {
    if (_spi != 0) {
        close(_spi);
    }
}

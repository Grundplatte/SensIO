//
// Created by Markus Feldbacher on 30.01.18.
//

#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "SPI_HAL.h"

int SPI_HAL::read(byte_t slave_addr, byte_t regAddr, unsigned int length, byte_t *data) {
    int ret;

    struct spi_ioc_transfer tr[1] = {0,};

    tr[0].tx_buf = (unsigned long) data;
    tr[0].len = length;

    ret = ioctl(_spi, SPI_IOC_MESSAGE(1), &tr);

    return ret;
}

int SPI_HAL::write(byte_t slave_addr, byte_t reg_addr, unsigned int length, byte_t *data) {

}

SPI_HAL::SPI_HAL() {
    if ((_spi = open(SPI_INTERFACE_0, O_RDWR)) < 0)
        exit(EXIT_FAILURE);

    // Set SPI parameters.
    if (ioctl(_spi, SPI_IOC_WR_MODE, 0) < 0)
        exit(EXIT_FAILURE);

    if (ioctl(_spi, SPI_IOC_WR_BITS_PER_WORD, 8) < 0)
        exit(EXIT_FAILURE);

    if (ioctl(_spi, SPI_IOC_WR_MAX_SPEED_HZ, SPI_1MHZ) < 0)
        exit(EXIT_FAILURE);
}

SPI_HAL::~SPI_HAL() {
    if (_spi != 0) {
        close(_spi);
    }
}

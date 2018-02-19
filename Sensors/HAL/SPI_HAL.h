//
// Created by Markus Feldbacher on 30.01.18.
//

#ifndef SIDECHANNEL_SPI_HAL_H
#define SIDECHANNEL_SPI_HAL_H

#include "HAL.h"
#include "wiringPiSPI.h"

// Raspi has 3 SPI controllers. Header = SPI0
#define SPI_INTERFACE "/dev/spidev0.0"

class SPI_HAL : HAL {
public:
    int read(byte slave_addr, byte regAddr, unsigned int length, byte *data) override;

    int write(byte slave_addr, byte reg_addr, unsigned int length, byte *data) override;


    SPI_HAL();

    ~SPI_HAL();

private:
    int _spi;
};


#endif //SIDECHANNEL_SPI_HAL_H

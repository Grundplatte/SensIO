/**
    SensIO
    SPI_HAL.h

    Implementation of SPI HAL. This was programmed for a raspberry pi, which has multiple SPI busses. The bus can be
    changed by altering the SPI_INTERFACE definition.


    @todo Address translation; wip
    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_SPI_HAL_H
#define SIDECHANNEL_SPI_HAL_H

#include "HAL.h"
#include "wiringPiSPI.h"

// Raspi has multiple SPI controllers. Header = SPI0
#define SPI_INTERFACE "/dev/spidev0.0"
//#define SPI_INTERFACE "/dev/spidev0.1"

#define SPI_1MHZ 1000000
#define SPI_10MHZ 10000000

class SPI_HAL : public HAL {
public:
    int read(byte_t slave_addr, byte_t regAddr, unsigned int length, byte_t *data) override;

    int write(byte_t slave_addr, byte_t reg_addr, unsigned int length, byte_t *data) override;

    SPI_HAL();

    ~SPI_HAL();

private:
    int _spi;
};


#endif //SIDECHANNEL_SPI_HAL_H

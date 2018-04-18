/**
    SensIO
    NoEDC.h

    Dummy class used to disable the EDC feature.

    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_NOEDC_H
#define SIDECHANNEL_NOEDC_H

#include "EDC.h"

class NoEDC : public EDC {
public:
    NoEDC();

    ~NoEDC();

    int generate(std::vector<bit_t> input, std::vector<bit_t> &output) override;

    int check(std::vector<bit_t> input, std::vector<bit_t> edc_in) override;

    int calcOutputSize(unsigned int length) override;
};

#endif // SIDECHANNEL_NOEDC_H
/**
    SensIO
    Berger.h

    Simple implementation of the Berger code.

    @author Markus Feldbacher
*/

#ifndef SIDECHANNEL_BERGER_H
#define SIDECHANNEL_BERGER_H

#include <bitset>
#include "EDC.h"

class Berger: public EDC {
public:
    Berger();

    ~Berger();

    /*
     * Generate a berger code from the input
     * input: payload
     * output: payload + berger code
     * length: input length in byte
     * return: output length in byte
     */
    int generate(std::vector<bit_t> input, std::vector<bit_t> &output) override;

    /*
     * Validate the payload + berger code
     * input: payload + berger code
     * length: input length in byte
     * return: valid (0) or invalid(-1)
     */
    int check(std::vector<bit_t> input, std::vector<bit_t> edc_in) override;

    int calcOutputSize(unsigned int length) override;
};

#endif // SIDECHANNEL_BERGER_H
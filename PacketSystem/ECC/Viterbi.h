#pragma once
#include <iostream>
#include <vector>
#include <math.h>
#include <bitset>  

#include "../../Defines.h"
//#include "ECC.h"

/**
 * Class intended for use and testing of the Convolutional encoding and decoding using the Viterbi algorithm
 * Settings (num. of input, outputs and constraint) are set in Defines.h
 * Encoder is set to respond to the different number of inputs, Decoder is not.
 * Testing and both Encoder and Decoder are done with the number of outputs being 2 in mind (ideally, code radio is 1/2)
 * Program should adapt to the number of the shift registers (K, constraint number)
*/

// Constraint level (number of shift registers, not including the input ones)
#define K 2
// Number of input ports of encoder
#define k 1
// Number of output ports of encoder
#define n 2

// TODO: rework
class Viterbi
{
public:
	// <------------- CODING FUNCTIONS ------------->
	/**
	* desc. : encode the input sequence with the convolutional encoding
	* input : input - input bit sequence (each element is 0 or 1)
	* output : vector<byte> - encoded sequence
	*/
	std::vector<byte_t> encode(std::vector<byte_t> input);

	/**
	* desc. : decode the input sequence with the convolutional decoding using the Viterbi principal
	* input : input - input byte sequence, where each byte corresponds to one output by the encoder (hence, max. 8 outputs in this way)
	* output : vector<byte> - decoded sequence
	*/
	std::vector<byte_t> decode(std::vector<byte_t> input);


	// <------------- CONSTRUCTOR AND DESTRUCTOR ------------->

	Viterbi();
	~Viterbi();

private:

	// <------------- HELP FUNCTIONS ------------->

	/**
	 * desc. : get the next state from the state_table vector depending on the current state and input
	 * input : state - current state of the register, input - the desired input
	 * output : byte - the next sequential state from the table
	*/
	byte_t getNextState(byte_t state, byte_t input);

	/**
	* desc. : get the next output of the encoder based on the state of registers and input
	* input : state - current state of the register, input - the desired input
	* output : byte - the output of the encoder (the predefined table)
	*/
	byte_t getOutput(byte_t state, byte_t input);

	/**
	* desc. : calculate the hamming distance between two bytes on the bit level
	* input : val1 - first byte, val2 - second byte
	* output : byte - resulted hamming distance value
	*/
	byte_t calcHammingDistance(byte_t val1, byte_t val2);

	/**
	* desc. : writes out the program information
	* input : none
	* output : none
	*/
	void startProcess();

	/// Global state table for keeping the next sequential state of the current state depending on the input
	std::vector<std::vector<byte_t > > state_table;

	/// Global output table for keeping the outputs of the current state depending on the input (* currently, only for n = 2 outputs programmed)
	std::vector<std::vector<byte_t > > output_table;
	
	/// Global table that gives inputs based on the current state and the sequential state (which is influenced by the defined input)
	std::vector<std::vector<byte_t > > input_table;

};


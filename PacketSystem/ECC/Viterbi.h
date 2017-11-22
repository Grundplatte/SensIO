#pragma once
#include <iostream>
#include <vector>
#include <math.h>
#include <bitset>  

#include "../../Defines.h"
#include "ECC.h"

/**
 * Class intended for use and testing of the Convolutional encoding and decoding using the Viterbi algorithm
 * Settings (num. of input, outputs and constraint) are set in Defines.h
 * Encoder is set to respond to the different number of inputs, Decoder is not.
 * Testing and both Encoder and Decoder are done with the number of outputs being 2 in mind (ideally, code radio is 1/2)
 * Program should adapt to the number of the shift registers (K, constraint number)
*/

class Viterbi : ECC
{
public:
	// <------------- CODING FUNCTIONS ------------->
	/**
	* desc. : encode the input sequence with the convolutional encoding
	* input : input - input bit sequence (each element is 0 or 1)
	* output : vector<byte> - encoded sequence
	*/
	std::vector<byte> encode(std::vector<byte> input);

	/**
	* desc. : decode the input sequence with the convolutional decoding using the Viterbi principal
	* input : input - input byte sequence, where each byte corresponds to one output by the encoder (hence, max. 8 outputs in this way)
	* output : vector<byte> - decoded sequence
	*/
	std::vector<byte> decode(std::vector<byte> input);


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
	byte getNextState(byte state, byte input);

	/**
	* desc. : get the next output of the encoder based on the state of registers and input
	* input : state - current state of the register, input - the desired input
	* output : byte - the output of the encoder (the predefined table)
	*/
	byte getOutput(byte state, byte input);

	/**
	* desc. : calculate the hamming distance between two bytes on the bit level
	* input : val1 - first byte, val2 - second byte
	* output : byte - resulted hamming distance value
	*/
	byte calcHammingDistance(byte val1, byte val2);

	/**
	* desc. : writes out the program information
	* input : none
	* output : none
	*/
	void startProcess();

	/// Global state table for keeping the next sequential state of the current state depending on the input
	std::vector<std::vector<byte > > state_table;

	/// Global output table for keeping the outputs of the current state depending on the input (* currently, only for n = 2 outputs programmed)
	std::vector<std::vector<byte > > output_table;
	
	/// Global table that gives inputs based on the current state and the sequential state (which is influenced by the defined input)
	std::vector<std::vector<byte > > input_table;

};


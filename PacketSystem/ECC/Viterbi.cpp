#include "Viterbi.h"

using namespace std;

void Viterbi::startProcess()
{
    cout << endl << "||||||||||||||||||||||||||||||||||||||||" << endl;
    cout << "|||||Viterbi Convolutional Decoder||||||" << endl;
    cout << "|||||||||TU Graz - Fikret Basic|||||||||" << endl,
            cout << "||||||||||||||||||||||||||||||||||||||||" << endl;
}


//The constructor sets the three tables (private variables) depending on the configuration values (from the Defines.h).
//For the ease of the process, no G() polynominal corrections are made (optimisation is set for K = 3)

Viterbi::Viterbi()
{
	// Because it is not possible to use define values direct on operations, they are written separately.
	byte kPrim = k;
	byte kOne = K;
	byte counter = 0;

	if (kPrim < 1)
	{
		kPrim = 1;
	}
		
	if (kOne < 2)
	{
		kOne = 2;
	}

	startProcess();

    cout << "\nNum. of inputs: " << (int) kPrim;
    cout << "\nNum. of outputs: " << n;
    cout << "\nConstraint level: " << (int) kOne << "\n\n";
		
	// The loop goes through every row (state) 2^(K-1) and updates it based on the shift of every specific input (2^k)
	for(size_t i = 0; i < pow(2, kOne); i++)
	{
        vector<byte> temp;
		
		temp.resize(pow(2, kOne));
		for (size_t j = 0; j < pow(2, kOne); j++)
		{
			temp.at(j) = 'n';
		}
		input_table.push_back(temp);

        temp = vector<byte>();

		for (size_t j = 0; j < pow(2, kPrim); j++)
		{
			byte inp_val = j;
			inp_val = inp_val << kOne;
			inp_val = inp_val | counter;
			inp_val = inp_val >> kPrim;

			temp.push_back(inp_val);

			input_table.at(i).at(inp_val) = j;
		}

		counter++;
		state_table.push_back(temp);
	}

	// @TODO: Upgrade to be able to handle variable sized outputs

	// .... to write ...
	for (byte i = 0; i < pow(2, kOne); i++)
	{
        vector<byte> temp;

		for (byte j = 0; j < pow(2, kPrim); j++)
		{
			byte tempVal1 = j;
			byte tempVal2 = j;

			byte tempInner = tempVal1;

			for (byte t = 0; t < kPrim - 1; t++)
			{
				tempVal1 = (tempInner >> t & 1) ^ (tempInner >> (t + 1) & 1);
				tempVal2 = (tempInner >> t & 1) ^ (tempInner >> (t + 1) & 1);
			}

			for(byte t = 0; t < kOne; t++)
			{
				tempVal1 ^= (i >> t) & 1;
				
				if(t != 1)
				{
					tempVal2 ^= (i >> t) & 1;
				}
			}

			temp.push_back((tempVal1 << 1) | tempVal2);
		}

		output_table.push_back(temp);
	}

	
	// <------------------- cout testing ------------------->

    cout << "\nSequence State table" << endl << endl;

    cout << "   ";

	for (byte j = 0; j < pow(2, kPrim); j++)
	{
        cout << (int) j << " ";
    }

    cout << endl;

	for (byte i = 0; i < pow(2, kOne); i++)
	{
        cout << (int) i << ": ";

		for (byte j = 0; j < pow(2, kPrim); j++)
		{
            cout << (int) state_table.at(i).at(j) << " ";
		}

        cout << endl;
	}


    cout << "\nOutput table" << endl << endl;

    cout << "   ";

	for (byte j = 0; j < pow(2, kPrim); j++)
	{
        cout << (int) j << " ";
    }

    cout << endl;

	for (byte i = 0; i < pow(2, kOne); i++)
	{
        cout << (int) i << ": ";

		for (byte j = 0; j < pow(2, kPrim); j++)
		{
            cout << (int) output_table.at(i).at(j) << " ";
		}

        cout << endl;
	}


    cout << "\nInput value table" << endl << endl;

    cout << "    ";

	for (byte j = 0; j < pow(2, kOne); j++)
	{
        cout << (int) j << "  ";
    }

    cout << endl;

	for (byte i = 0; i < pow(2, kOne); i++)
	{
        cout << (int) i << " : ";

		for (byte j = 0; j < pow(2, kOne); j++)
		{
            cout << (int) input_table.at(i).at(j) << " ";
		}

        cout << endl;
	}

	//for (byte i = 0; i < pow(2, kOne); i++)
	//{
	//	for (byte j = 0; j < pow(2, kPrim); j++)
	//	{
    //		bitset<8> bitTestS{ i };
    //		bitset<8> bitTestB{ j };
    //		bitset<8> bitTest1{ output_table.at(i).at(j) };
    //		cout << "State: " << bitTestS << " , Input bit: " << bitTestB << " , Next state: " << bitTest1 << endl;
	//	}
    //	cout << endl;
	//}
}

byte Viterbi::getNextState(byte state, byte input)
{
	return state_table.at(state).at(input);
}

byte Viterbi::getOutput(byte state, byte input)
{
	return output_table.at(state).at(input);
}

byte Viterbi::calcHammingDistance(byte val1, byte val2)
{
	byte distance = 0;

	byte rez = val1 ^ val2;

	for (size_t i = 0; i < 8; i++)
	{
		byte temp;

		temp = (rez >> i) & 1;

		if (temp == 1)
		{
			distance++;
		}
	}

	return distance;
}

// Takes in consideration that the input is a sequence of bits, where the output will be the same (hence, on the pre-encoder side, values need
// to be transfored so that each byte of the vector is actually a bit)
vector<byte> Viterbi::encode(vector<byte> input)
{
	byte state = 0;
    vector<byte> encoded;

	for (size_t i = 0; i < input.size(); i++)
	{
		encoded.push_back(getOutput(state, input.at(i)));
		state = getNextState(state, input.at(i));
	}

	// Adding trailling (flush) zero bits
	while(state != 0)
	{
		encoded.push_back(getOutput(state, 0));
		state = getNextState(state, 0);
	}

    cout << "Encoded output: ";

	for (size_t i = 0; i < encoded.size(); i++)
	{
        cout << (int) encoded.at(i) << " ";
    }

    cout << "\n\n";

	return encoded;
}


vector<byte> Viterbi::decode(vector<byte> input) {
    vector<vector<byte> > error_metric;
    vector<vector<byte> > state_history;

	error_metric.resize(output_table.size());
	state_history.resize(output_table.size());

	for(size_t i = 0; i < output_table.size(); i++)
	{
		error_metric.at(i).resize(input.size());
		state_history.at(i).resize(input.size() +  1);
	}

    vector<byte> states(1);
    vector<byte> tempVec;

	/* First, it is necessary to set two tables, one for error metric and the other for previous state history.
	   Average error rate is calculated with the hamming distance, and always checked with the conflicting sate sequentions.  */

	for (size_t i = 0; i < input.size(); i++)
	{
        tempVec = vector<byte>();

        vector<bit> inputVerification(error_metric.size());

		for (size_t j = 0; j < states.size(); j++)
		{
			for(size_t t = 0; t < output_table.at(0).size(); t++)
			{
				byte tempState = getNextState(states.at(j), t);
				byte tempOutput = getOutput(states.at(j), t);

				byte hamming = calcHammingDistance(tempOutput, input.at(i));
				byte accumulated_metric = hamming;
				
				if(i != 0)
				{
					accumulated_metric += error_metric.at(states.at(j)).at(i - 1);
				}

				if(inputVerification.at(tempState))
				{
					if(error_metric.at(tempState).at(i) > accumulated_metric)
					{
						state_history.at(tempState).at(i + 1) = states.at(j);
						error_metric.at(tempState).at(i) = accumulated_metric;				
					}
					else if(error_metric.at(tempState).at(i) == hamming)
					{
						
					}
				}
				else
				{
					inputVerification.at(tempState) = true;

					state_history.at(tempState).at(i + 1) = states.at(j);
					error_metric.at(tempState).at(i) = accumulated_metric;

					tempVec.push_back(tempState);
				}
			}
		}

		states.resize(tempVec.size());
		states = tempVec;
	}

	// <------------ Traceback ------------>

    vector<byte> state_sequence(input.size() + 1);
    vector<byte> decoded(input.size());

	byte min = error_metric.at(0).at(error_metric.at(0).size() - 1);
	byte minPos = 0;

	for(size_t i = 1; i < error_metric.size(); i++)
	{
		if(error_metric.at(i).at(error_metric.at(0).size() - 1) < min)
		{
			min = error_metric.at(i).at(error_metric.at(0).size() - 1);
			minPos = i;
		}
	}

	state_sequence.at(state_sequence.size() - 1) = minPos;

	for(int i = state_sequence.size() - 2; i >= 0; i--)
	{
		state_sequence.at(i) = state_history.at(state_sequence.at(i+1)).at(i+1);
	}

	for(size_t i = 0; i < state_sequence.size() - 1; i++)
	{
		decoded.at(i) = input_table.at(state_sequence.at(i)).at(state_sequence.at(i + 1));
	}


	// <------------ Outputs for testing. Uncomment to run them. ------------>

	for (size_t i = 0; i < state_history.size(); i++)
	{
        cout << "State history: " << (int) i << "   ";

		for(size_t j = 0; j < state_history.at(i).size(); j++)
		{
            cout << (int) state_history.at(i).at(j) << "  ";
		}

        cout << "\n\n";
    }

    cout << "\n";

	for (size_t i = 0; i < error_metric.size(); i++)
	{
        cout << "Average metric cost: " << (int) i << "   ";

		for (size_t j = 0; j < error_metric.at(i).size(); j++)
		{
            cout << (int) error_metric.at(i).at(j) << "  ";
		}

        cout << "\n\n";
    }

    cout << endl << "Decoded sequence: ";

	for (size_t i = 0; i < decoded.size(); i++)
	{
        cout << (int) decoded.at(i) << "  ";
    }

    //cout << endl;

	return decoded;
}

Viterbi::~Viterbi()
{
}

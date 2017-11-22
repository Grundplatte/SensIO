#include "PacketManager.h"
#include "ECC/Hadamard.h"
#include "EDC/EDC.h"
#include "EDC/Berger.h"
#include "ECC/NoECC.h"
#include "EDC/NoEDC.h"

/*
 * 1) Wait for sync (no data for one tick)
 * 2) Receive Request
 */
 int PacketManager::waitForRequest(byte *sqnHad)
 {
	int i, bit;
	struct timespec start, stop;
	double accum;

     sqnHad = 0;
	for(i=7; i>=0; i--) {	
			
		m_sens->waitForSensReady();
		clock_gettime(CLOCK_REALTIME, &start);
		// wait until someone accesses the sensor results
		do {
			bit = m_sens->tryReadBit();
			//printf("[D] receive: bit = 0x%02x\n", bit);
			
			clock_gettime(CLOCK_REALTIME, &stop);
			accum = stop.tv_nsec - start.tv_nsec;
			// rollover
			if(accum < 0) {
				accum += 1000000000;
			}
			
			// timeout > delay ms
			if(i!=7 && accum > (MAXDELAY%1000) * 1000000) {
				return -1;
			}
		} while(bit < 0);
		
		if(bit){
			*sqnHad |= (1 << i);
			//printf("[D] receive: received 1\n");
		}
		else{
			//printf("[D] receive: received 0\n");
		}
	}
	
	return 0;
 }

/*
 * Request packet sqn
 */
 int PacketManager::request(unsigned int sqn)
 {
	 byte sqnByte, sqnHad;

     sqnByte = (byte)sqn%8;
	 m_ECC->encode(&sqnByte, 8, &sqnHad);

     // TODO: check
	 send(&sqnHad, 1);
 }

// send one packet (12bit data, 3bit sqn, 4bit edc = 19bit)
// memory is allocated inside the function, must be freed by caller after use
int PacketManager::create(byte *data, unsigned int length, byte **output)
{
	if(length == 0)
		return -1;
	
	printf("[D] createPackets: size %i\n", length);
	unsigned int numPackets = (length*8 - 1)/12 + 1;
	*output = (uint8_t *)malloc(numPackets * 3 * sizeof(byte));
	memset(*output, 0, numPackets * 3 * sizeof(byte));
	
	unsigned int i;
	uint32_t dataindex, dataoffset;
	for(i=0; i<numPackets; i++){
		// 12bits data
		if(!i%2){
			(*output)[i*3 + 0] = data[i + 0]; // byte 1
			(*output)[i*3 + 1] = (data[i + 1] & 0xF0); // upper 4 bits; byte 2, bit 1-4
		}
		else{
			(*output)[i*3 + 0] = (data[i + 0] & 0xF) << 4; // lower 4 bits; byte 1, bit 1-4
			(*output)[i*3 + 0] |= (data[i + 1] & 0xF0) >> 4; // upper 4 bits; byte 1, bit 5-8
			(*output)[i*3 + 1] = (data[i + 1] & 0xF) << 4; // lower 4 bits; byte 2, bit 1-4
		}
		
		//printf(">>sqn: %i\n", i);
		//printf(">>data: 0x%x 0x%x\n", (*output)[i + 0], (*output)[i + 1]);
		
		// sqn
		(*output)[i*3 + 1] |= (i & 0x7) << 1; // byte 1, bit 5-7
		
		// generate edc (Berger)
		byte berger;
		m_EDC->generate((*output)+(i*3), 2, &berger);
		//printf(">>berger: 0x%x\n", berger);
		(*output)[i*3 + 1] |= (berger & 0x08) >> 3; // byte 1, bit 8
		(*output)[i*3 + 2] = (berger & 0x07) << 5; // byte 2, bit 1-3
		
		//printf("> PacketManager: 0x%x 0x%x 0x%x\n", (*output)[i*3 + 0], (*output)[i*3 + 1], (*output)[i*3 + 2]);
	}
	return numPackets;
}

int PacketManager::unpack(byte *packets, unsigned int length, byte **output)
{
	// TODO: implement
}

// send packet (19bit/3byte)
int PacketManager::send(byte *packets, unsigned int index)
{
	unsigned int l, start, stop;
	int i;
	byte bit;
	struct timespec req, rem;

	start = index*3;
	stop = start + 2;
	
	bit = 0x01;
	req.tv_sec = 0;
	req.tv_nsec = 10000000; // 10ms

	for(l=start; l<=stop; l++){
		for(i=7; i>=0; i--) {	
			// wait until the sensor is ready
			m_sens->waitForSensReady();
			m_sens->sendBit(packets[l] & (bit << i));
			
			// stop after 3bits => send only 19bits not 24
			if(l==stop && i==5)
				break;
			
			nanosleep(&req, &rem);
		}
		printf("[D] send: sent 0x%02x\n", packets[l]);
	}
}

int PacketManager::receive(byte *packet)
{
	unsigned int l;
	int bit, i;
	struct timespec start, stop;
	double accum;

	for(l=0; l<3; l++){
		packet[l] = 0;
		
		for(i=7; i>=0; i--) {		
			m_sens->waitForSensReady();
			clock_gettime(CLOCK_REALTIME, &start);
			// wait until someone accesses the sensor results
			do {
				bit = m_sens->tryReadBit();
				//printf("[D] receive: bit = 0x%02x\n", bit);
				
				clock_gettime(CLOCK_REALTIME, &stop);
				accum = stop.tv_nsec - start.tv_nsec;
				// rollover
				if(accum < 0) {
					accum += 1000000000;
				}
				
				// timeout > delay ms
				if(accum > (MAXDELAY%1000) * 1000000) {
					return -1;
				}
			} while(bit < 0);
			
			if(bit){
				packet[l] |= (1 << i);
				//printf("[D] receive: received 1\n", packet[l]);
			}
			else{
				//printf("[D] receive: received 0\n", packet[l]);
			}
			
			if(l==2 && i==5)
				break;
		}
		printf("[D] receive: received 0x%02x\n", packet[l]);
	}
	return 0;
}

int PacketManager::check(byte *packet, unsigned int seq)
{
    byte berger_pack;
    byte berger_calc;
	
	// extract berger code
	berger_pack = 0;
	berger_pack |= (packet[1] & 0x01) << 3;
	berger_pack |= (packet[2] & 0xE0) >> (8-3);
	
	// calc berger code from information
    byte *packet_copy;
	packet_copy = (byte *)malloc(3 * sizeof(byte));
	memcpy(packet_copy, packet, 3 * sizeof(byte));
	
	packet_copy[1] &= 0xFE;
	m_EDC->generate(packet_copy, 2, &berger_calc);
	
	free(packet_copy);
	
	// check if 
	printf("[D] Berger: 0x%x 0x%x\n", berger_pack, berger_calc);
}

PacketManager::PacketManager() {
    // m_ECC = new Hadamard();
    m_ECC = new NoECC();

    //m_EDC = new Berger();
    m_EDC = new NoEDC();

    m_sens = new HTS221();
}

PacketManager::~PacketManager() {
    delete(m_EDC);
    delete(m_ECC);
    delete(m_sens);
}

/*
Copyright (c) 2014-2015 NicoHood
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// Include guard
#pragma once

//================================================================================
// Protocol Definitions
//================================================================================

//PANASONIC
//IRP notation: {37k,432}<1,-1|1,-3>(8,-4,3:8,1:8,D:8,S:8,F:8,(D^S^F):8,1,-173)+
// Lead + Space logic
#define PANASONIC_HZ				37000
#define PANASONIC_PULSE				432UL
#define PANASONIC_BLOCKS			6
#define PANASONIC_ADDRESS_LENGTH	16
#define PANASONIC_COMMAND_LENGTH	32
#define PANASONIC_LENGTH			(2 + PANASONIC_BLOCKS * 8 * 2) // 2 for lead + space, each block has 8bits: mark and space
#define PANASONIC_TIMEOUT			(PANASONIC_PULSE * 173)
#define PANASONIC_MARK_LEAD			(PANASONIC_PULSE * 8)
#define PANASONIC_SPACE_LEAD		(PANASONIC_PULSE * 4)
#define PANASONIC_SPACE_HOLDING		0 // no holding function in this protocol
#define PANASONIC_MARK_ZERO			(PANASONIC_PULSE * 1)
#define PANASONIC_MARK_ONE			(PANASONIC_PULSE * 1)
#define PANASONIC_SPACE_ZERO		(PANASONIC_PULSE * 1)
#define PANASONIC_SPACE_ONE			(PANASONIC_PULSE * 3)

/*
Panasonic pulse demonstration:

*---------|                        |------------|   |---------|   |---|   ... -|   |---------
*         |                        |            |   |         |   |   |   ...  |   |
*         |                        |            |   |         |   |   |   ...  |   |
*         |------------------------|            |---|         |---|   |-  ...  |---|
*         |       Lead Mark        | Lead Space |  Logical 1  | Log 0 |  Data  |End|

*/

//================================================================================
// Decoding Class
//================================================================================

class Panasonic : public CIRLData{
public:
	Panasonic(void){
		// Empty	
	}
	
	// Hide anything that is inside this class so the user dont accidently uses this class
	template<uint32_t debounce, typename protocol, typename ...protocols>
	friend class CIRLremote;
	
private:
	static inline uint8_t getSingleFlag(void) __attribute__((always_inline));
	static inline bool available(void) __attribute__((always_inline));
	static inline void read(IR_data_t* data) __attribute__((always_inline));

	// Decode functions for a single protocol/multiprotocol for less/more accuration
	static inline void decodeSingle(const uint16_t &duration, const uint32_t &debounce) __attribute__((always_inline));
	static inline void decode(const uint16_t &duration, const uint32_t &debounce) __attribute__((always_inline));

protected:
	// Temporary buffer to hold bytes for decoding the protocols
	// not all of them are compiled, only the used ones
	static uint8_t countPanasonic;
	static uint8_t dataPanasonic[PANASONIC_BLOCKS];
};


uint8_t Panasonic::getSingleFlag(void){
	return FALLING;
}


bool Panasonic::available(void)
{
	// Only return a value if this protocol has new data
	if(IRLProtocol == IR_PANASONIC)
		return true;
	else
		return false;	
}


void Panasonic::read(IR_data_t* data){
	// Only (over)write new data if this protocol received any data
	if(available()){
		data->address = UINT16_AT_OFFSET(dataPanasonic, 0);
		data->command = UINT32_AT_OFFSET(dataPanasonic, 2);
		data->protocol = IRLProtocol;
		
		// Reset protocol for new reading
		IRLProtocol &= ~IR_NEW_PROTOCOL;
	}
}


void Panasonic::decodeSingle(const uint16_t &duration, const uint32_t &debounce){
	// no accuracy set at the moment, no conflict detected yet
	// due to the checksum we got a good recognition
	const uint8_t irLength = PANASONIC_LENGTH / 2;
	const uint16_t timeoutThreshold = (PANASONIC_TIMEOUT + PANASONIC_MARK_LEAD + PANASONIC_SPACE_LEAD) / 2;
	const uint16_t leadThreshold = (PANASONIC_MARK_LEAD + PANASONIC_SPACE_LEAD + PANASONIC_MARK_ONE + PANASONIC_SPACE_ONE) / 2;
	const uint16_t threshold = (PANASONIC_MARK_ONE + PANASONIC_MARK_ZERO + PANASONIC_SPACE_ONE + PANASONIC_SPACE_ZERO) / 2;

	// if timeout always start next possible reading and abort any pending readings
	if (duration >= timeoutThreshold)
		countPanasonic = 0;

	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
	// this is to not conflict with other protocols while they are sending 0/1
	// which might be similar to a lead in this protocol
	else if (countPanasonic == 0)
		return;

	// check Mark Lead (needs a timeout or a correct signal)
	else if (countPanasonic == 1) {
		// wrong lead
		if (duration < leadThreshold) {
			countPanasonic = 0;
			return;
		}
		// else normal lead, next reading
	}

	// pulses (mark + space)
	else {
		// check different logical space pulses

		// get number of the Space Bits (starting from zero)
		// only save every 2nd value, substract the first two lead pulses
		uint8_t length = countPanasonic - 2;

		// move bits and write 1 or 0 depending on the duration
		// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
		dataPanasonic[length / 8] >>= 1;
		// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
		if (duration >= threshold)
			dataPanasonic[length / 8] |= 0x80;


		// last mark (stop bit)
		if (countPanasonic >= irLength) {
			// reset reading
			countPanasonic = 0;

			// Check if the protcol's checksum is correct
			if (uint8_t(dataPanasonic[2] ^ dataPanasonic[3] ^ dataPanasonic[4]) == dataPanasonic[5])
				// check vendor parity nibble (optional)
				//uint8_t XOR = data[0] ^ data[1];
				//if ((XOR & 0x0F ^ (XOR >> 4)) != 0x00)
				//	return 0;
				// new input, now check for debounce
				IRLProtocol = IR_PANASONIC;

			return;
		}
	}

	// next reading, no errors
	countPanasonic++;
}


void Panasonic::decode(const uint16_t &duration, const uint32_t &debounce) {
	// no accuracy set at the moment, no conflict detected yet
	// due to the checksum we got a good recognition
	const uint8_t irLength = PANASONIC_LENGTH;
	const uint16_t timeoutThreshold = (PANASONIC_TIMEOUT + PANASONIC_MARK_LEAD) / 2;
	const uint16_t markLeadThreshold = (PANASONIC_MARK_LEAD + PANASONIC_SPACE_ONE) / 2;
	const uint16_t spaceLeadThreshold = (PANASONIC_SPACE_LEAD + PANASONIC_SPACE_ONE) / 2;
	const uint16_t spaceThreshold = (PANASONIC_SPACE_ONE + PANASONIC_SPACE_ZERO) / 2;

	// if timeout always start next possible reading and abort any pending readings
	if (duration >= timeoutThreshold)
		countPanasonic = 0;

	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
	// this is to not conflict with other protocols while they are sending 0/1
	// which might be similar to a lead in this protocol
	else if (countPanasonic == 0)
		return;

	// check Mark Lead (needs a timeout or a correct signal)
	else if (countPanasonic == 1) {
		// wrong lead
		if (duration < markLeadThreshold) {
			countPanasonic = 0;
			return;
		}
	}

	//check Space Lead/Space Holding
	else if (countPanasonic == 2) {
		// wrong space
		if (duration < spaceLeadThreshold) {
			countPanasonic = 0;
			return;
		}
		// else normal Space, next reading
	}

	// last mark (stop bit)
	else if (countPanasonic > irLength) {
		// reset reading
		countPanasonic = 0;

		// Check if the protcol's checksum is correct
		if (uint8_t(dataPanasonic[2] ^ dataPanasonic[3] ^ dataPanasonic[4]) == dataPanasonic[5])
			// check vendor parity nibble (optional)
			//uint8_t XOR = data[0] ^ data[1];
			//if ((XOR & 0x0F ^ (XOR >> 4)) != 0x00)
			//	return 0;
			// new input, now check for debounce
			IRLProtocol = IR_PANASONIC;

		return;
	}

	// Space pulses (even numbers)
	else if (countPanasonic % 2 == 0) {
		// check different logical space pulses

		// get number of the Space Bits (starting from zero)
		// only save every 2nd value, substract the first two lead pulses
		uint8_t length = (countPanasonic / 2) - 2;

		// move bits and write 1 or 0 depending on the duration
		// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
		dataPanasonic[length / 8] >>= 1;
		// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
		if (duration >= spaceThreshold)
			dataPanasonic[length / 8] |= 0x80;
	}
	// Mark pulses (odd numbers)
	// else ignored, always the same mark length

	// next reading, no errors
	countPanasonic++;
}

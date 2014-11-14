/*
Copyright (c) 2014 NicoHood
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

#ifndef IRLREMOTE_H
#define IRLREMOTE_H

#include <Arduino.h>
#include <util/delay_basic.h>

//================================================================================
// Definitions
//================================================================================

//NEC
//IRP notation: {38.4k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,-78,(16,-4,1,-173)*)
// Lead + Space logic
#define NEC_HZ 38400
#define NEC_PULSE 564UL
#define NEC_BLOCKS 4
#define NEC_LENGTH 2 + NEC_BLOCKS*8*2 // 2 for lead + space, each block has 8bits: mark and space
#define NEC_TIMEOUT NEC_PULSE*173
#define NEC_MARK_LEAD NEC_PULSE*16
#define NEC_SPACE_LEAD NEC_PULSE*8
#define NEC_SPACE_HOLDING NEC_PULSE*4
#define NEC_MARK_ZERO NEC_PULSE*1
#define NEC_MARK_ONE NEC_PULSE*1
#define NEC_SPACE_ZERO NEC_PULSE*1
#define NEC_SPACE_ONE NEC_PULSE*3

//PANASONIC
//IRP notation: {37k,432}<1,-1|1,-3>(8,-4,3:8,1:8,D:8,S:8,F:8,(D^S^F):8,1,-173)+
// Lead + Space logic
#define PANASONIC_HZ 37000
#define PANASONIC_PULSE 432UL
#define PANASONIC_BLOCKS 6
#define PANASONIC_LENGTH 2 + PANASONIC_BLOCKS*8*2 // 2 for lead + space, each block has 8bits: mark and space
#define PANASONIC_TIMEOUT PANASONIC_PULSE*173
#define PANASONIC_MARK_LEAD PANASONIC_PULSE*8
#define PANASONIC_SPACE_LEAD PANASONIC_PULSE*4
#define PANASONIC_SPACE_HOLDING 0 // no holding function in this protocol
#define PANASONIC_MARK_ZERO PANASONIC_PULSE*1
#define PANASONIC_MARK_ONE PANASONIC_PULSE*1
#define PANASONIC_SPACE_ZERO PANASONIC_PULSE*1
#define PANASONIC_SPACE_ONE PANASONIC_PULSE*3

/*
Panasonic pulse demonstration:

*---------|                        |------------|   |---------|   |---|   ... -|   |---------
*         |                        |            |   |         |   |   |   ...  |   |
*         |                        |            |   |         |   |   |   ...  |   |
*         |------------------------|            |---|         |---|   |-  ...  |---|
*         |       Lead Mark        | Lead Space |  Logical 1  | Log 0 |  Data  |End|

*/

//SONY 8, 12, 15, 20
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:8,^22200)
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:7,D:5,^45m)+ 
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:7,D:8,^45m)+ 
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:7,D:5,S:8,^45m) +
// Lead + Mark logic
#define SONY_HZ 40000
#define SONY_PULSE 600UL
#define SONY_BLOCKS_8 1
#define SONY_BLOCKS_12 2
#define SONY_BLOCKS_15 2
#define SONY_BLOCKS_20 3
#define SONY_LENGTH_8 2 + (8-1)*2 // 2 for lead + space, -1 for mark end, 8 bit
#define SONY_LENGTH_12 2 + (7+5-1)*2 // 2 for lead + space, -1 for mark end, 12 bit
#define SONY_LENGTH_15 2 + (7+8-1)*2 // 2 for lead + space, -1 for mark end, 15 bit
#define SONY_LENGTH_20 2 + (7+5+8-1)*2 // 2 for lead + space, -1 for mark end, 20 bit
#define SONY_TIMEOUT_8 22200
#define SONY_TIMEOUT 45000 // 12, 15, 20 have the same timeout
#define SONY_MARK_LEAD SONY_PULSE*4
#define SONY_SPACE_LEAD SONY_PULSE*1
#define SONY_SPACE_HOLDING 0 // no holding function in this protocol
#define SONY_MARK_ZERO SONY_PULSE*1
#define SONY_MARK_ONE SONY_PULSE*2
#define SONY_SPACE_ZERO SONY_PULSE*1
#define SONY_SPACE_ONE SONY_PULSE*1

typedef enum IRType{
	IR_NO_PROTOCOL, // 0
	IR_USER, // 1
	IR_ALL, // 2
	IR_NEC, // ...
	IR_PANASONIC,
	IR_SONY8,
	IR_SONY12,
	IR_SONY15,
	IR_SONY20,
	// add new protocols here
	IR_RAW,
};

// definitions to use decoding functions with extra accuracy
#define IR_EXTRA_ACCURACY true
#define IR_NO_EXTRA_ACCURACY false

// definition to get the higher value
#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

// definition to get the lower value
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

//================================================================================
// Prototypes
//================================================================================

// attach the interrupt function
template <IRType irType>
inline void IRLbegin(const uint8_t interrupt);

// dettach the interrupt function
inline void IRLend(const uint8_t interrupt);

// variables for IR processing if no user function was set
extern uint8_t  IRLProtocol;
extern uint16_t IRLAddress;
extern uint32_t IRLCommand;

// functions to use if no user function was set
inline bool IRLavailable(void);
inline void IRLreset(void);
inline uint8_t IRLgetProtocol(void);
inline uint16_t IRLgetAddress(void);
inline uint32_t IRLgetCommand(void);

// function called on a valid IR event, must be overwritten by the user
void __attribute__((weak)) IREvent(uint8_t protocol, uint16_t address, uint32_t command);

// optinal user function called when the user wants to implement its own decoding function
void __attribute__((weak)) decodeIR(const uint32_t duration);

// called by interrupt CHANGE
template <IRType irType>
inline void IRLinterrupt(void);

// special decode function for each protocol
inline void decodeAll(const uint32_t duration);
template <bool extraAccuracy> inline void decodeNec(const uint32_t duration);
template <bool extraAccuracy> inline void decodePanasonic(const uint32_t duration);
template <bool extraAccuracy> inline void decodeSony8(const uint32_t duration);
template <bool extraAccuracy> inline void decodeSony12(const uint32_t duration);
template <bool extraAccuracy> inline void decodeSony15(const uint32_t duration);
template <bool extraAccuracy> inline void decodeSony20(const uint32_t duration);
inline void decodeRaw(const uint32_t duration);

// functions to check if the received data is valid with the protocol checksums
inline bool IRLcheckInverse0(uint8_t data[]);
inline bool IRLcheckInverse1(uint8_t data[]);
inline bool IRLcheckHolding(uint8_t data[]);
inline bool IRLcheckXOR0(uint8_t data[]);

// default decoder helper function
template <uint8_t irLength, uint32_t timeoutThreshold, uint16_t markLeadThreshold, uint16_t spaceLeadThreshold,
	uint16_t spaceLeadHoldingThreshold, uint16_t markThreshold, uint16_t spaceThreshold,
	uint16_t markTimeout, uint16_t spaceTimeout>
	inline bool IRLdecode(uint32_t duration, uint8_t data[]);

// functions to send the protocol
//TODO template
void IRLwrite(const uint8_t pin, uint16_t address, uint32_t command);
void IRLwriteNEC(volatile uint8_t * outPort, uint8_t bitmask, uint16_t address, uint32_t command);

// functions to set the pin high or low (with bitbang pwm)
void IRLmark(const uint16_t Hz, volatile uint8_t * outPort, uint8_t bitMask, uint16_t time);
void IRLspace(volatile uint8_t * outPort, uint8_t bitMask, uint16_t time);

//================================================================================
// Inline Implementations
//================================================================================

template <IRType irType>
void IRLbegin(const uint8_t interrupt){
	// attach the function that decodes the signals
	attachInterrupt(interrupt, IRLinterrupt<irType>, CHANGE);
}

void IRLend(const uint8_t interrupt){
	// release the interrupt, if its NOT_AN_INTERRUPT the detach function does nothing
	detachInterrupt(interrupt);
	// also make sure to call reset() after end() if you use the intern reading functions
}

bool IRLavailable(void){
	return IRLProtocol;
}

void IRLreset(void){
	IRLProtocol = 0;
}

uint8_t IRLgetProtocol(void){
	return IRLProtocol;
}

uint16_t IRLgetAddress(void){
	return IRLAddress;
}

uint32_t IRLgetCommand(void){
	return IRLCommand;
}

template <IRType irType>
void IRLinterrupt(void){
	//save the duration between the last reading
	static uint32_t lastTime = 0;
	uint32_t time = micros();
	uint32_t duration = time - lastTime;
	lastTime = time;

	// determinate which decode function must be called
	switch (irType){
	case IR_USER:
		// userfunction that can be overwritten
		decodeIR(duration);
		break;
	case IR_ALL:
		decodeAll(duration);
		break;
	case IR_NEC:
		decodeNec<IR_NO_EXTRA_ACCURACY>(duration);
		break;
	case IR_PANASONIC:
		decodePanasonic<IR_NO_EXTRA_ACCURACY>(duration);
		break;
	//case IR_SONY8:
	//	decodeSony8<IR_NO_EXTRA_ACCURACY>(duration);
	//	break;
	case IR_SONY12:
		decodeSony12<IR_NO_EXTRA_ACCURACY>(duration);
		break;
	//case IR_SONY15:
	//	decodeSony15<IR_NO_EXTRA_ACCURACY>(duration);
	//	break;
	//case IR_SONY20:
	//	decodeSony20<IR_NO_EXTRA_ACCURACY>(duration);
	//	break;
	case IR_RAW:
		decodeRaw(duration);
		break;
	}
}

void decodeAll(const uint32_t duration){
	// go through all known protocols and decode with more (resource unfriendly) accuration
	decodeNec<IR_EXTRA_ACCURACY>(duration);
	decodePanasonic<IR_EXTRA_ACCURACY>(duration);
	decodeSony12<IR_EXTRA_ACCURACY>(duration);
}

template <bool extraAccuracy> void decodeNec(const uint32_t duration){
	// temporary buffer to hold bytes for decoding this protocol
	static uint8_t data[NEC_BLOCKS];
	// pass the duration to the decoding function
	if (IRLdecode <NEC_LENGTH, (NEC_TIMEOUT + NEC_MARK_LEAD) / 2, // irLength, timeoutThreshold
		(NEC_MARK_LEAD + NEC_SPACE_ONE) / 2, (NEC_SPACE_LEAD + NEC_SPACE_HOLDING) / 2, // markLeadThreshold, spaceLeadThreshold
		(NEC_SPACE_HOLDING + NEC_SPACE_ONE) / 2, 0, // spaceLeadHoldingThreshold, markThreshold
		(NEC_SPACE_ONE + NEC_SPACE_ZERO) / 2, // spaceThreshold
		0, 0>// markTimeout, spaceTimeout
		(duration, data)){
		// Check if the protcol's checksum is correct
		// In some other Nec Protocols the Address has an inverse or not, so we only check the command
		if (IRLcheckInverse1(data) || IRLcheckHolding(data)){
			// you could check the address for inverse0 too
			// but newer devices use an extended address without inverse
			uint16_t address = (data[0] << 8) | data[1];
			uint32_t command = ((data[2] << 8) | data[3]) & 0xFFFF;
			IREvent(IR_NEC, address, command);
			return;
		}
	}
}

template <bool extraAccuracy> void decodePanasonic(const uint32_t duration){
	// temporary buffer to hold bytes for decoding this protocol
	static uint8_t data[PANASONIC_BLOCKS];
	// pass the duration to the decoding function
	if (IRLdecode <PANASONIC_LENGTH, (PANASONIC_TIMEOUT + PANASONIC_MARK_LEAD) / 2, // irLength, timeoutThreshold
		(PANASONIC_MARK_LEAD + PANASONIC_SPACE_ONE) / 2, (PANASONIC_SPACE_LEAD + PANASONIC_SPACE_ONE) / 2, // markLeadThreshold, spaceLeadThreshold
		0, 0, // spaceLeadHoldingThreshold, markThreshold
		(PANASONIC_SPACE_ONE + PANASONIC_SPACE_ZERO) / 2, // spaceThreshold
		0, 0>// markTimeout, spaceTimeout
		(duration, data)){
		// Check if the protcol's checksum is correct
		if (IRLcheckXOR0(data)){
			uint16_t address = (data[0] << 8) | data[1];
			uint32_t command = (uint32_t(data[2]) << 24) | (uint32_t(data[3]) << 16) | (uint32_t(data[4]) << 8) | data[5];
			IREvent(IR_PANASONIC, address, command);
			return;
		}
	}
}

template <bool extraAccuracy> void decodeSony12(const uint32_t duration){
	// temporary buffer to hold bytes for decoding this protocol
	static uint8_t data[SONY_BLOCKS_12];

		// pass the duration to the decoding function
	bool newInput;
	if (extraAccuracy)
		newInput = IRLdecode <SONY_LENGTH_12, (SONY_TIMEOUT + SONY_MARK_LEAD) / 2, // irLength, timeoutThreshold
		(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, 0, // markLeadThreshold, spaceLeadThreshold
		0, (SONY_MARK_ONE + SONY_MARK_ZERO) / 2, // spaceLeadHoldingThreshold, markThreshold
		0, // spaceThreshold
		(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, SONY_MARK_ONE>// markTimeout, spaceTimeout
		(duration, data);
	else
		newInput = IRLdecode <SONY_LENGTH_12, (SONY_TIMEOUT + SONY_MARK_LEAD) / 2, // irLength, timeoutThreshold
		(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, 0, // markLeadThreshold, spaceLeadThreshold
		0, (SONY_MARK_ONE + SONY_MARK_ZERO) / 2, // spaceLeadHoldingThreshold, markThreshold
		0, // spaceThreshold
		0, 0>// markTimeout, spaceTimeout
		(duration, data);

	if (newInput){
		// protocol has no checksum
		//TODO LSB
		uint16_t address = ((data[0] & 0x01) << 4) | data[1] & 0x0F;
		uint32_t command = data[0] >> 1;
		IREvent(IR_SONY12, address, command);
		return;
	}
}

void decodeRaw(const uint32_t duration){
	//TODO, use advanced raw example instead
}

bool IRLcheckInverse0(uint8_t data[]){
	// check if byte 2 and is the inverse of byte 3
	if (uint8_t((data[0] ^ (~data[1]))) == 0)
		return true;
	else return false;
}

bool IRLcheckInverse1(uint8_t data[]){
	// check if byte 0 and is the inverse of byte 1
	if (uint8_t((data[2] ^ (~data[3]))) == 0)
		return true;
	else return false;
}

bool IRLcheckHolding(uint8_t data[]){
	if (data[2] == 0xFF && data[3] == 0xFF)
		return true;
	else return false;
}

bool IRLcheckXOR0(uint8_t data[]){
	// this function is used for panasonic checksum
	if (uint8_t(data[2] ^ data[3] ^ data[4]) == data[5])
		return true;
	else return false;
}

template <uint8_t irLength, uint32_t timeoutThreshold, uint16_t markLeadThreshold, uint16_t spaceLeadThreshold,
	uint16_t spaceLeadHoldingThreshold, uint16_t markThreshold, uint16_t spaceThreshold,
	uint16_t markTimeout, uint16_t spaceTimeout>
	bool IRLdecode(uint32_t duration, uint8_t data[]){

	// variables for ir processing
	static uint8_t count = 0;

	// if timeout always start next possible reading and abort any pending readings
	if (duration >= timeoutThreshold)
		count = 1;

	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
	// this is to not conflict with other protocols while they are sending 0/1 which might be similar to a lead in another protocol
	else if (count == 0)
		return false;

	// check pulses for mark/space and lead + logical 0/1 seperate
	else{
		// Mark pulses (odd numbers)
		if (count % 2 == 1){
			// check Mark Lead (needs a timeout or a correct signal)
			if (markLeadThreshold && count == 1){
				// lead is okay// wrong lead
				if (duration <= markLeadThreshold){
					count = 0;
					return false;
				}
			}

			else{
				// check for timeout if needed (might be a different protocol)
				if (markTimeout && duration > markTimeout){
					count = 0;
					return false;
				}

				// only check values if the protocol has different logical space pulses
				else if (markThreshold){

					// get number of the Mark Bits (starting from zero)
					uint8_t length;
					// only save every 2nd value, substract the first two lead pulses
					if (!spaceThreshold)
						length = (count / 2) - 1;
					// special case: spaces and marks both have data in the pulse
					else length = count - 2;

					// move bits and write 1 or 0 depending on the duration
					data[length / 8] <<= 1;
					if (duration > markThreshold)
						data[length / 8] |= 0x01;
					else // normally not needed through the bitshift
						data[length / 8] &= ~0x01;
				}

				// check last input (always a mark)
				if (count > irLength){
					count = 0;
					return true;
				}
			}
		}

		// Space pulses (even numbers)
		else{
			//check Space Lead/Space Holding
			if (spaceLeadThreshold && count == 2){
					// normal Space, next reading
					if (duration > spaceLeadThreshold);

					// Button holding (if supported by protocol)
					else if (spaceLeadHoldingThreshold && duration > spaceLeadHoldingThreshold){
						// set command to 0xFF if button is held down
						if (irLength == (4 * 8 * 2 + 2)){
							data[0] = data[1] = 0x00;
							data[2] = data[3] = 0xFF;
						}
						count = 0;
						return true;
					}
					// wrong space
					else {
						count = 0;
						return false;
					}
			}
			else{
				// check for timeout if needed (might be a different protocol)
				if (spaceTimeout && duration > spaceTimeout){
					count = 0;
					return false;
				}

				// only check values if the protocol has different logical space pulses
				else if (spaceThreshold){

					// get number of the Space Bits (starting from zero)
					uint8_t length;
					// only save every 2nd value, substract the first two lead pulses
					if (!markThreshold)
						length = (count / 2) - 2;
					// special case: spaces and marks both have data in the pulse
					else length = count - 2;

					// move bits and write 1 or 0 depending on the duration
					data[length / 8] <<= 1;
					if (duration > spaceThreshold)
						data[length / 8] |= 0x01;
					else // normally not needed through the bitshift
						data[length / 8] &= ~0x01;
				}

			}
		}

		// next reading, no errors
		count++;
	}

	// no valid input (yet)
	return false;
}

#endif
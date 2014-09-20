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
#define PANASONIC_HZ 37000
#define PANASONIC_PULSE 432UL
#define PANASONIC_BLOCKS 6
#define PANASONIC_LENGTH 2 + PANASONIC_BLOCKS*8*2 //2 for lead + space, each block has 8bits: mark and space
#define PANASONIC_TIMEOUT PANASONIC_PULSE*173
#define PANASONIC_MARK_LEAD PANASONIC_PULSE*8
#define PANASONIC_SPACE_LEAD PANASONIC_PULSE*4
#define PANASONIC_SPACE_HOLDING 0 // no holding function in this protocol
#define PANASONIC_MARK_ZERO PANASONIC_PULSE*1
#define PANASONIC_MARK_ONE PANASONIC_PULSE*1
#define PANASONIC_SPACE_ZERO PANASONIC_PULSE*1
#define PANASONIC_SPACE_ONE PANASONIC_PULSE*3

typedef enum IRType{
	IR_NO_PROTOCOL, // 0
	IR_USER, // 1
	IR_ALL, // 2
	IR_NEC, // ...
	IR_PANASONIC,
	// add new protocols here
	IR_RAW,
};

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
inline uint8_t IRLgetProtocol(void);
inline uint16_t IRLgetAddress(void);
inline uint32_t IRLgetCommand(void);
inline void IRLreset(void);

// function called on a valid IR event, must be overwritten by the user
void __attribute__((weak)) IREvent(uint8_t protocol, uint16_t address, uint32_t command);
void __attribute__((weak)) decodeIR(const uint32_t duration);

// called by interrupt CHANGE
template <IRType irType>
inline void IRLinterrupt(void);

// special decode function for each protocol
inline void decodeAll(const uint32_t duration);
inline void decodeNec(const uint32_t duration);
inline void decodePanasonic(const uint32_t duration);
inline void decodeRaw(const uint32_t duration);

// functions to check if the received data is valid with the protocol checksums
inline bool IRLcheckInverse0(uint8_t data[]);
inline bool IRLcheckInverse1(uint8_t data[]);
inline bool IRLcheckHolding(uint8_t data[]);
inline bool IRLcheckXOR0(uint8_t data[]);

// default decoder helper function
template <uint32_t timeout, uint16_t markLead, uint16_t spaceLead, uint16_t spaceHolding,
	uint16_t spaceZero, uint16_t spaceOne, uint16_t irLength, uint8_t blocks>
	inline bool IRLdecodeSpace(unsigned long duration, uint8_t data[]);

// functions to send the protocol
//TODO template
void IRLwrite(const uint8_t pin, uint16_t address, uint32_t command);
void IRLwriteNEC(volatile uint8_t * outPort, uint8_t bitmask, uint16_t address, uint32_t command);

// functions to set the pin high or low (with bitbang pwm)
void mark(const uint16_t Hz, volatile uint8_t * outPort, uint8_t bitMask, uint16_t time);
void space(volatile uint8_t * outPort, uint8_t bitMask, uint16_t time);

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

uint8_t IRLgetProtocol(void){
	return IRLProtocol;
}

uint16_t IRLgetAddress(void){
	return IRLAddress;
}

uint32_t IRLgetCommand(void){
	return IRLCommand;
}

void IRLreset(void){
	IRLProtocol = 0;
}

template <IRType irType>
void IRLinterrupt(void){
	//save the duration between the last reading
	static unsigned long lastTime = 0;
	unsigned long time = micros();
	unsigned long duration = time - lastTime;
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
		decodeNec(duration);
		break;
	case IR_PANASONIC:
		decodePanasonic(duration);
		break;
	case IR_RAW:
		decodeRaw(duration);
		break;
	}
}

void decodeAll(const uint32_t duration){
	decodeNec(duration);
	decodePanasonic(duration);
}

void decodeNec(const uint32_t duration){
	// temporary buffer to hold bytes for decoding this protocol
	static uint8_t data[NEC_BLOCKS];
	// pass the duration to the decoding function
	if (IRLdecodeSpace <NEC_TIMEOUT, NEC_MARK_LEAD, NEC_SPACE_LEAD, NEC_SPACE_HOLDING,
		NEC_SPACE_ZERO, NEC_SPACE_ONE, NEC_LENGTH, NEC_BLOCKS>
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

void decodePanasonic(const uint32_t duration){
	// temporary buffer to hold bytes for decoding this protocol
	static uint8_t data[PANASONIC_BLOCKS];
	// pass the duration to the decoding function
	if (IRLdecodeSpace <PANASONIC_TIMEOUT, PANASONIC_MARK_LEAD, PANASONIC_SPACE_LEAD, PANASONIC_SPACE_HOLDING,
		PANASONIC_SPACE_ZERO, PANASONIC_SPACE_ONE, PANASONIC_LENGTH, PANASONIC_BLOCKS>
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

template <uint32_t timeout, uint16_t markLead, uint16_t spaceLead, uint16_t spaceHolding,
	uint16_t spaceZero, uint16_t spaceOne, uint16_t irLength, uint8_t blocks>
	bool IRLdecodeSpace(unsigned long duration, uint8_t data[]){
	// variables for ir processing
	static uint8_t count = 0;

	// if timeout(start next value)
	if (duration >= ((timeout + markLead) / 2))
		count = 0;

	// check Lead (needs a timeout or a correct signal)
	else if (count == 0){
		// lead is okay
		if (duration > ((markLead + spaceLead) / 2))
			count++;
		// wrong lead
		else count = 0;
	}

	//check Space/Space Holding
	else if (count == 1){
		// protocol supports space holding (Nec)
		if (spaceHolding){
			// normal Space
			if (duration > (spaceLead + spaceHolding) / 2)
				// next reading
				count++;

			// Button holding
			else if (duration > (spaceHolding + spaceOne) / 2){
				// set command to 0xFF if button is held down
				if (blocks <= 4){
					data[0] = data[1] = 0x00;
					data[2] = data[3] = 0xFF;
				}
				count = 0;
				return true;
			}
			// wrong space
			else count = 0;
		}

		// protocol doesnt support space holding (Panasonic)
		else{
			// normal Space
			if (duration > (spaceLead + spaceOne) / 2)
				count++;
			// wrong space
			else count = 0;
		}
	}

	// High pulses (odd numbers)
	else if (count % 2 == 1){
		// get number of the High Bits minus one for the lead
		uint8_t length = (count / 2) - 1;

		// move bits and write 1 or 0 depending on the duration
		data[length / 8] <<= 1;
		if (duration > ((spaceOne + spaceZero) / 2))
			data[length / 8] |= 0x01;
		else
			data[length / 8] &= ~0x01;

		// next reading
		count++;
	}

	// Low pulses (even numbers)
	else{
		// You dont really need to check them for errors.
		// But you might miss some wrong values
		// Checking takes more operations but is safer.
		// We want maximum recognition so we leave this out here.
		// also we have the inverse or the XOR to check the data later
		count++;
	}

	// check last input
	// TODO calculate with blocks to not go over bounds
	if (count >= irLength){
		count = 0;
		return true;
	}
	return false;
}

#endif
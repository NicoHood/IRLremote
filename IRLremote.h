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

//NEC
//IRP notation: {38.4k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,-78,(16,-4,1,-173)*) 
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

//================================================================================
// Typedefs
//================================================================================

typedef enum IRType{
	IR_ALL,
	IR_NEC,
	IR_PANASONIC,
};

// typedef for ir signal types
typedef union{
	uint8_t whole[6];
	struct{
		uint16_t address;
		uint32_t command;
	};
} IR_Remote_Data_t;

//================================================================================
// Prototypes
//================================================================================

// attach the interrupt function
inline void IRLbegin(const uint8_t interrupt);

// dettach the interrupt function
inline void IRLend(const uint8_t interrupt);

// variables for IR processing if no user function was set
extern bool newInput;
extern IR_Remote_Data_t lastIRData;

// function called on a valid IR event, must be overwritten by the user
void __attribute__((weak)) irEvent(IR_Remote_Data_t IRData);

// functions to use if no user function was set
bool IRLavailable(void);
IR_Remote_Data_t IRLread(void);

// called by interrupt CHANGE
void IRLinterrupt(void);

// default decoder helper function
template <uint32_t timeout, uint16_t markLead, uint16_t spaceLead, uint16_t spaceHolding,
	uint16_t spaceZero, uint16_t spaceOne, uint16_t irLength, uint8_t blocks>
	uint8_t* IRLdecodeSpace(unsigned long duration);




// functions to send the protocol
//TODO template
//void write(const uint8_t pin, IR_Remote_Data_t IRData);
//void writeNEC(const uint8_t pin, IR_Remote_Data_t IRData);

//TODO remove this out of here
//uint8_t _bitMask;
//volatile uint8_t * _outPort;	

// functions to set the pin high or low (with bitbang pwm)
void mark38_4(int time);
void mark37(int time);
void space(int time);

//================================================================================
// Implementation
//================================================================================

void IRLbegin(const uint8_t interrupt){
	// attach the function that decodes the signals
	attachInterrupt(interrupt, IRLinterrupt, CHANGE);
}

void IRLend(const uint8_t interrupt){
	// release the interrupt, if its NOT_AN_INTERRUPT the detach function does nothing
	detachInterrupt(interrupt);

	// ensure available(), doesnt return anything
	//TODO move out here?
	newInput = false;
}

#endif
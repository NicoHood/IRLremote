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

#ifndef IRLremote_h
#define IRLremote_h

#include <Arduino.h>

// Select your protocol here
// information about protocols:
// http://www.hifi-remote.com/johnsfine/DecodeIR.html

#define NEC
//#define PANASONIC 


//NEC
//IRP notation: {38.4k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,-78,(16,-4,1,-173)*) 
#if defined(NEC)
#define IR_PULSE 564
#define IR_BLOCKS 4
#define IR_LEAD IR_PULSE*16
#define IR_SPACE IR_PULSE*8
#define IR_SPACE_HOLDING IR_PULSE*4
#define IR_LOW_0 IR_PULSE*1
#define IR_LOW_1 IR_PULSE*1
#define IR_HIGH_0 IR_PULSE*1
#define IR_HIGH_1 IR_PULSE*3
#define IR_LENGTH 2 + IR_BLOCKS*8*2 //2 for lead&space, each block has 8bits: low and high
#define IR_TIMEOUT IR_PULSE*173/2
typedef union{
	uint8_t whole[4];
	struct{
		uint16_t address;
		uint16_t command;
	};
} IR_Remote_Data_t;

#endif


//PANASONIC
//IRP notation: {37k,432}<1,-1|1,-3>(8,-4,3:8,1:8,D:8,S:8,F:8,(D^S^F):8,1,-173)+ 
#if defined(PANASONIC)
#define IR_PULSE 432
#define IR_BLOCKS 6
#define IR_LEAD IR_PULSE*8
#define IR_SPACE IR_PULSE*4
#define IR_SPACE_HOLDING IR_SPACE //no holding
#define IR_LOW_0 IR_PULSE*1
#define IR_LOW_1 IR_PULSE*1
#define IR_HIGH_0 IR_PULSE*1
#define IR_HIGH_1 IR_PULSE*3
#define IR_LENGTH 2 + IR_BLOCKS*8*2 //2 for lead&space, each block has 8bits: low and high
#define IR_TIMEOUT IR_PULSE*173/2

typedef union{
	uint8_t whole[6];
	struct{
		uint16_t address;
		uint32_t command;
	};
} IR_Remote_Data_t;

#endif

class CIRLremote{
public:
	CIRLremote(void);

	// set userfunction to access new input directly
	void begin(uint8_t interrupt, void(*function)(IR_Remote_Data_t) = NULL);
	void end(void);

	// functions if no user function was set
	bool available(void);
	IR_Remote_Data_t read(void);

private:
	// interrupt function with rapper to use static + virtual at the same time
	static void interruptIR_wrapper(void);
	void interruptIR(void);
	virtual bool decodeIR(unsigned long duration); //TODO=0

	// function called on a valid IR event
	void(*user_onReceive)(IR_Remote_Data_t);

	// variables for ir processing
	uint8_t mInterrupt;
	IR_Remote_Data_t IRData;
	bool pauseIR;
	unsigned long  mLastTime;
	uint8_t mCount;

	void* k;

};

extern CIRLremote IRLremote;

#endif



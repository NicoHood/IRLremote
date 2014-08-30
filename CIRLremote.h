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

#ifndef CIRLREMOTE_H
#define CIRLREMOTE_H

#include <Arduino.h>

// typedef for ir signal types
typedef union{
	uint8_t whole[6];
	struct{
		uint16_t address;
		uint32_t command;
	};
} IR_Remote_Data_t;


// decoding class definition
class CIRLprotocol{
public:
	CIRLprotocol(void){ }

	// decode function and its reset call needs to be implemented
	virtual bool decodeIR(unsigned long duration) = 0;
	virtual void reset(void) = 0;

	// variables for ir processing
	IR_Remote_Data_t IRData;
};


//typedef enum IRType{
//	NEC			= 1,
//	PANASONIC	= 2,
//};


//NEC
//IRP notation: {38.4k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,-78,(16,-4,1,-173)*) 
#define NEC_PULSE 564
#define NEC_BLOCKS 4
#define NEC_MARK_LEAD NEC_PULSE*16
#define NEC_SPACE_LEAD NEC_PULSE*8
#define NEC_SPACE_HOLDING NEC_PULSE*4
#define NEC_MARK_ZERO NEC_PULSE*1
#define NEC_MARK_ONE NEC_PULSE*1
#define NEC_SPACE_ZERO NEC_PULSE*1
#define NEC_SPACE_ONE NEC_PULSE*3
#define NEC_LENGTH 2 + NEC_BLOCKS*8*2 // 2 for lead + space, each block has 8bits: mark and space
#define NEC_TIMEOUT NEC_PULSE*173/2 // we check half of the documented timeout

// ir management class
class CIRLremote{
public:
	CIRLremote(void);

	// set userfunction to access new input directly
	void begin(uint8_t interrupt, CIRLprotocol &protocol, void(*function)(IR_Remote_Data_t) = NULL);
	void end(void);

	// functions if no user function was set
	bool available(void);
	IR_Remote_Data_t read(void);

	void write(const uint8_t pin, IR_Remote_Data_t IRData);
	void writeNEC(const uint8_t pin, IR_Remote_Data_t IRData);

	// functions to set the pin high (with bitbang pwm) or low
	void mark38_4(int time);
	void mark37(int time);
	void space(int time);

	// additional functions to control the lib
	//unsigned long getTimeout(void);
	//bool paused(void);
	//void pause(void);
	//void resume(void);
	//void reset(void);

private:
	// interrupt function with rapper to use static + virtual at the same time
	static CIRLremote *active_object;
	static void interruptIR_wrapper(void);
	void interruptIR(void);

	// function called on a valid IR event
	void(*user_onReceive)(IR_Remote_Data_t);

	// ir managment variables
	uint8_t mInterrupt;
	bool newInput;
	//bool pauseIR;
	unsigned long  mLastTime;

	CIRLprotocol* IRprotocol;

	uint8_t _bitMask;
	volatile uint8_t * _outPort;
};


extern CIRLremote IRLremote;

#endif
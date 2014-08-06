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

	//virtual void write(void* data, uint8_t length); //TODO = 0;

	// variables for ir processing
	IR_Remote_Data_t IRData;
};


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

private:
	// interrupt function with rapper to use static + virtual at the same time
	static CIRLremote *active_object;
	static void interruptIR_wrapper(void);
	void interruptIR(void);

	// function called on a valid IR event
	void(*user_onReceive)(IR_Remote_Data_t);

	// ir managment variables
	uint8_t mInterrupt;
	bool pauseIR;
	unsigned long  mLastTime;

	CIRLprotocol* IRprotocol;
};

extern CIRLremote IRLremote;

#endif
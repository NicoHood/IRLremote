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

#ifndef IRLPROTOCOLPANASONIC_h
#define IRLPROTOCOLPANASONIC_h

#include <CIRLremote.h>

//PANASONIC
//IRP notation: {37k,432}<1,-1|1,-3>(8,-4,3:8,1:8,D:8,S:8,F:8,(D^S^F):8,1,-173)+ 
#define IR_PULSE 432
#define IR_BLOCKS 6
#define IR_LEAD IR_PULSE*8
#define IR_SPACE IR_PULSE*4
#define IR_LOW_0 IR_PULSE*1
#define IR_LOW_1 IR_PULSE*1
#define IR_HIGH_0 IR_PULSE*1
#define IR_HIGH_1 IR_PULSE*3
#define IR_LENGTH 2 + IR_BLOCKS*8*2 //2 for lead&space, each block has 8bits: low and high
#define IR_TIMEOUT IR_PULSE*173/2


class IRLprotocolPanasonic : public CIRLprotocol{
public:
	IRLprotocolPanasonic(void){ }

	// virtual functions that needs to be implemented:
	bool decodeIR(unsigned long duration);
	void reset(void);

private:
	uint8_t mCount;
};

#endif

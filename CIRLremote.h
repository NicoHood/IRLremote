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
#include <avr/pgmspace.h>

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

//template <IRType irType>
//class CIRLprotocol2{
//public:
//	CIRLprotocol2(void){ }
//
//};
//1904 53

// decoding class definition
class CIRLprotocol{
public:
	CIRLprotocol(IRType type):irType(type){ }
	const uint8_t irType; //TODO save this byte
	bool decodeIR(unsigned long duration){
		switch (irType){
		case IR_NEC:
			uint8_t * data = decodeSpace < NEC_TIMEOUT, NEC_MARK_LEAD, NEC_SPACE_LEAD, NEC_SPACE_HOLDING,
				NEC_SPACE_ZERO, NEC_SPACE_ONE, NEC_LENGTH, NEC_BLOCKS >
				(duration);
			if (data){

				// In some other Nec Protocols the Address has an inverse or not, so we only check the command
				if (uint8_t((data[2] ^ (~data[3]))) == 0){
					// Errorcorrection for the Command is the inverse
					memcpy(IRData.whole, data, NEC_BLOCKS);
					IRData.whole[4] = 0;
					IRData.whole[5] = 0;

					if (uint8_t((data[0] ^ (~data[1]))) == 0){
						// normal NEC with mirrored address
					} // else extended NEC

					return true;
				}
				//else if (IRData.command == -1L)
				//	return true;
			}
			return false;
		}
	};


	inline void reset(void){
		//TODO improve
		// sends a timeout, function should reset
		decodeIR(-1L);
	}

	// default decoder helper functions
	template <uint16_t timeout, uint16_t markLead, uint16_t spaceLead, uint16_t spaceHolding,
		uint16_t spaceZero, uint16_t spaceOne, uint16_t irLength, uint8_t blocks>
		uint8_t* decodeSpace(unsigned long duration){
		// variables for ir processing
		static uint8_t data[blocks];
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
					memset(data, 0xFF, blocks);
					count = 0;
					return data;
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
		if (count >= irLength){
			count = 0;
			return data;
		}
		return NULL;
	}

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

	void write(const uint8_t pin, IR_Remote_Data_t IRData);
	void writeNEC(const uint8_t pin, IR_Remote_Data_t IRData);

	// functions to set the pin high (with bitbang pwm) or low
	void mark38_4(int time);
	void mark37(int time);
	void space(int time);

	IR_Remote_Data_t IRData;

	// additional functions to control the lib
	//unsigned long getTimeout(void);
	//bool paused(void);
	//void pause(void);
	//void resume(void);
	//void reset(void);

private:
	// interrupt function with rapper to use static + virtual at the same time
	//TODO remove active object and replace with IRLremote instance
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

	//TODO remove this out of here
	uint8_t _bitMask;
	volatile uint8_t * _outPort;
};


extern CIRLremote IRLremote;

#endif
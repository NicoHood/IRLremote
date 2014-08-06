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

#include "IRLremote.h"

// instantiate object of CIRLremote
CIRLremote IRLremote;

// assign global variable which is used in the static wrapper function
void* pt2CIRLremoteObject = (void*)&IRLremote;

CIRLremote::CIRLremote(void){
	// ensure available() doesnt return anything
	pauseIR = false;
	// if someone calls end() without calling begin() first or twice
	mInterrupt = NOT_AN_INTERRUPT;

	k = &IRLremote;
}

void CIRLremote::begin(uint8_t interrupt, void(*function)(IR_Remote_Data_t)){
	// check if we already set an Interrupt, do not set it twice!
	if (mInterrupt != uint8_t(NOT_AN_INTERRUPT)) return;

	// reset variables
	pauseIR = false;
	mLastTime = 0;
	mCount = 0;

	// save new values
	mInterrupt = interrupt;
	user_onReceive = function;

	//TODO pullup? only for 1.5.7 with pin to interrupt define possible
	//pinMode(2, INPUT_PULLUP);

	attachInterrupt(mInterrupt, interruptIR_wrapper, CHANGE);
}

void CIRLremote::end(void){
	// release the interrupt, if its NOT_AN_INTERRUPT the detach function does nothing
	detachInterrupt(mInterrupt);

	// ensure available() doesnt return anything
	pauseIR = false;

	// if someone calls end() without calling begin() first or twice
	mInterrupt = NOT_AN_INTERRUPT;
}

bool CIRLremote::available(void){
	return pauseIR;
}

IR_Remote_Data_t CIRLremote::read(void){
	IR_Remote_Data_t IRReport;

	// reset remote and return data. Copy before unpausing!
	if (pauseIR){
		IRReport = IRData;
		pauseIR = false;
	}
	// return empty report
	else
		memset(&IRReport, 0, sizeof(IRReport));

	return IRReport;
}

void CIRLremote::interruptIR_wrapper(void){ //called interrupt CHANGE
	// explicitly cast to a pointer to Classname
	CIRLremote* mySelf = (CIRLremote*)pt2CIRLremoteObject;

	// call member
	mySelf->interruptIR();
}

void CIRLremote::interruptIR(void){ //called interrupt CHANGE
	// for no user function set we need to pause IR
	// to not overwrite the actual values until they are read.
	if (pauseIR) return;

	//save the duration between the last reading
	unsigned long time = micros();
	unsigned long duration = time - mLastTime;
	mLastTime = time;

	// check if we have a new input and call the userfunction
	// if no userfunction is set, flag a newinput signal.
	if (decodeIR(duration)){
		if (user_onReceive != NULL)
			user_onReceive(IRData);
		else
			pauseIR = true;
	}
}



bool CIRLremote::decodeIR(unsigned long duration){

	// if timeout(start next value)
	if (duration >= IR_TIMEOUT)
		mCount = 0;

	// check Lead (needs a timeout or a correct signal)
	else if (mCount == 0){
		// lead is okay
		if (duration > (IR_SPACE_HOLDING + IR_LEAD) / 2)
			mCount++;
		// wrong lead
		else mCount = 0;
	}

	//check Space/Space Holding
	else if (mCount == 1){
#if (IR_SPACE != IR_SPACE_HOLDING)
		// normal Space
		if (duration > (IR_SPACE + IR_SPACE_HOLDING) / 2)
			// next reading
			mCount++;

		// Button holding
		else if (duration > (IR_HIGH_1 + IR_SPACE_HOLDING) / 2){
			IRData.address = 0;
			IRData.command = 0xFFFFFFFF;
			mCount = 0;
			return true;
		}
#else //no Space Holding
		// normal Space
		if (duration > (IR_SPACE + IR_HIGH_1) / 2) mCount++; // next reading
#endif
		// wrong space
		else mCount = 0;
	}

	// High pulses (odd numbers)
	else if (mCount % 2 == 1){
		// get number of the High Bits
		// minus one for the lead
		uint8_t length = (mCount / 2) - 1;

		// write logical 1
		if (duration > (IR_HIGH_0 + IR_HIGH_1) / 2) IRData.whole[length / 8] |= (0x80 >> (length % 8));
		// write logical 0
		else IRData.whole[length / 8] &= ~(0x80 >> (length % 8));
		// next reading
		mCount++;
	}

	// Low pulses (even numbers)
	else{
		// You dont really need to check them for errors.
		// But you might miss some wrong values
		// Checking takes more operations but is safer.
		// We want maximum recognition so we leave this out here.
		// also we have the inverse or the XOR to check the data.

		// write low bits
		//if(duration>(IR_LOW_0+IR_LOW_1)/2);
		//else;

		//check for error
		//if(duration>(IR_HIGH_0+IR_HIGH_1)/2) mCount=0;
		//else
		mCount++;
	}

	// check last input
	if (mCount >= IR_LENGTH){
		/*
		//Debug output
		for(int i=0; i<IR_BLOCKS;i++){
		for(int j=0; j<8;j++){
		Serial.print((mHighBits[i]>>(7-j))&0x01);
		}
		Serial.println();
		}
		Serial.println();
		*/

		//write command based on each Protocol
#ifdef PANASONIC
		// Errorcorrection for Panasonic with XOR
		// Address left out, we dont need that (0,1)
		if (uint8_t(IRData.whole[2] ^ IRData.whole[3] ^ IRData.whole[4]) == IRData.whole[5]){
			//in my case [2] has a fix value
			uint32_t command = (IRData.whole[3] << 8) + IRData.whole[4];
			mCount = 0;
			return true;
		}
#endif

#ifdef NEC
		// You can also check the Address, but i wont do that.
		// In some other Nec Protocols the Address has an inverse or not
		if (uint8_t((IRData.whole[2] ^ (~IRData.whole[3]))) == 0){
			// Errorcorrection for the Command is the inverse
			uint32_t command = IRData.whole[2];
			mCount = 0;
			return true;
		}
#endif

		mCount = 0;
	}
	return false;
}

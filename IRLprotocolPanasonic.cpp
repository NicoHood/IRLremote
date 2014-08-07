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

#include <IRLprotocolPanasonic.h>

void IRLprotocolPanasonic::reset(void){
	mCount = 0;
}

bool IRLprotocolPanasonic::decodeIR(unsigned long duration){

	// if timeout(start next value)
	if (duration >= IR_TIMEOUT)
		mCount = 0;

	// check Lead (needs a timeout or a correct signal)
	else if (mCount == 0){
		// lead is okay
		if (duration > (IR_SPACE + IR_LEAD) / 2)
			mCount++;
		// wrong lead
		else mCount = 0;
	}

	//check Space
	else if (mCount == 1){
		// normal Space
		if (duration > (IR_SPACE + IR_HIGH_1) / 2) mCount++; // next reading

		// wrong space
		else mCount = 0;
	}

	// High pulses (odd numbers)
	else if (mCount % 2 == 1){
		// get number of the High Bits
		// minus one for the lead
		uint8_t length = (mCount / 2) - 1;

		// write logical 1
		if (duration > (IR_HIGH_0 + IR_HIGH_1) / 2) IRData.whole[length / 8] |= (1 << (length % 8));
		// write logical 0
		else IRData.whole[length / 8] &= ~(1 << (length % 8));
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

		// Errorcorrection for Panasonic with XOR
		if (uint8_t(IRData.whole[2] ^ IRData.whole[3] ^ IRData.whole[4]) == IRData.whole[5]){
			// in my case [2] has a fix value
			mCount = 0;
			return true;
		}

		mCount = 0;
	}
	return false;
}
///*
//Copyright (c) 2014 NicoHood
//See the readme for credit to other people.
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
//*/
//
//#include <IRLprotocolRaw.h>
//
//void IRLprotocolRaw::reset(void){
//	// reset counter on every new input
//	mCount = 0;
//}
//
//bool IRLprotocolRaw::decodeIR(unsigned long duration){
//	// block until data is read
//	if (available()) return false;
//
//	// save value and increase count
//	buffer[mCount++] = duration;
//
//	// save last time for timeout check in the buff too
//	if (mCount != IR_RAW_BUFFER_SIZE)
//		buffer[mCount] = micros();
//
//	// always return false to not call any interrupt function
//	return false;
//}
//
//uint8_t IRLprotocolRaw::available(void){
//	uint8_t oldSREG = SREG;
//	cli();
//
//	// check if we have data
//	if (mCount){
//		// check if buffer is full
//		if (mCount == IR_RAW_BUFFER_SIZE){
//			SREG = oldSREG;
//			return mCount;
//		}
//
//		// check if last reading was a timeout
//		// no problem if mCount==0 because the if above prevents this
//		// anyways then it will just return 0
//		if (buffer[mCount - 1] > IR_RAW_TIMEOUT){
//			SREG = oldSREG;
//			return mCount;
//		}
//
//		// check if reading timed out and save value
//		unsigned long duration = micros() - buffer[mCount];
//		if (duration > IR_RAW_TIMEOUT){
//			buffer[mCount++] = duration;
//			SREG = oldSREG;
//			return mCount;
//		}
//	}
//
//	SREG = oldSREG;
//	return 0;
//}
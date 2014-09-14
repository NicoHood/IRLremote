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

#include <CIRLremote.h>

//CIRLremote IRLremote;

//void IRLbegin(const uint8_t interrupt){
//	// attach the wrapper function that calls our main function on an interrupt
//	attachInterrupt(interrupt, IRLinterrupt, CHANGE);
//}

//void IRLinterrupt(void){
//	//save the duration between the last reading
//	static unsigned long lastTime = 0;
//	unsigned long time = micros();
//	unsigned long duration = time - lastTime;
//	lastTime = time;
//
//	// determinate which decode function must be called
//	IRType irType = IR_NEC;
//	switch (irType){
//	case IR_NEC:
//		uint8_t * data = IRLdecodeSpace <NEC_TIMEOUT, NEC_MARK_LEAD, NEC_SPACE_LEAD, NEC_SPACE_HOLDING,
//			NEC_SPACE_ZERO, NEC_SPACE_ONE, NEC_LENGTH, NEC_BLOCKS >
//			(duration);
//		if (data){
//
//			// In some other Nec Protocols the Address has an inverse or not, so we only check the command
//			if (uint8_t((data[2] ^ (~data[3]))) == 0){
//				IR_Remote_Data_t IRData;
//				// Errorcorrection for the Command is the inverse
//				memcpy(IRData.whole, data, NEC_BLOCKS);
//				IRData.whole[4] = 0;
//				IRData.whole[5] = 0;
//
//				if (uint8_t((data[0] ^ (~data[1]))) == 0){
//					// normal NEC with mirrored address
//				} // else extended NEC
//				irEvent(IRData);
//				return;
//			}
//			//else if (IRData.command == -1L)
//			//	return true;
//		}
//	}
//}


//void CIRLremote::begin(const uint8_t interrupt){
//	// attach the wrapper function that calls our main function on an interrupt
//	attachInterrupt(interrupt, interruptIR_wrapper, CHANGE);
//}

//void CIRLremote::interruptIR_wrapper(void){
//	// Called by interrupt CHANGE
//	// Call the instance. Needed for non static functions
//	//IRLremote.interruptIR();
//}

void CIRLremote::interruptIR(void){
	////save the duration between the last reading
	//unsigned long time = micros();
	//unsigned long duration = time - mLastTime;
	//mLastTime = time;

	//// determinate which decode function must be called
	//IRType irType = IR_NEC;
	//switch (irType){
	//case IR_NEC:
	//	uint8_t * data = IRLdecodeSpace <NEC_TIMEOUT, NEC_MARK_LEAD, NEC_SPACE_LEAD, NEC_SPACE_HOLDING,
	//		NEC_SPACE_ZERO, NEC_SPACE_ONE, NEC_LENGTH, NEC_BLOCKS >
	//		(duration);
	//	if (data){

	//		// In some other Nec Protocols the Address has an inverse or not, so we only check the command
	//		if (uint8_t((data[2] ^ (~data[3]))) == 0){
	//			IR_Remote_Data_t IRData;
	//			// Errorcorrection for the Command is the inverse
	//			memcpy(IRData.whole, data, NEC_BLOCKS);
	//			IRData.whole[4] = 0;
	//			IRData.whole[5] = 0;

	//			if (uint8_t((data[0] ^ (~data[1]))) == 0){
	//				// normal NEC with mirrored address
	//			} // else extended NEC
	//			irEvent(IRData);
	//			return;
	//		}
	//		//else if (IRData.command == -1L)
	//		//	return true;
	//	}
	//}
}

// ensure available() returns false
//static bool newInput = false;
//static IR_Remote_Data_t lastIRData = { 0 };
// Called when directly received correct IR Signal
// Do not use Serial inside, it can crash your Arduino!
void irEvent(IR_Remote_Data_t IRData) {
	//// For no user function set we need to pause IR
	//// to not overwrite the actual values until they are read.
	//if (newInput) return;

	//lastIRData = IRData;
	//newInput = true;
}


bool CIRLremote::available(void){
	//return newInput;
}

IR_Remote_Data_t CIRLremote::read(void){
	//// by default return an empty report
	//IR_Remote_Data_t IRReport = { 0 };

	//// get new data if available
	//if (newInput)
	//	IRReport = lastIRData;

	////unpause and return data
	//newInput = false;
	//return IRReport;
}


void CIRLremote::end(const uint8_t interrupt){
//	// release the interrupt, if its NOT_AN_INTERRUPT the detach function does nothing
//	detachInterrupt(interrupt);
//
//	// ensure available(), doesnt return anything
//	newInput = false;
}



//void CIRLremote::write(const uint8_t pin, IR_Remote_Data_t IRData)
//{
//	// Get the port mask and the pointers to the out/mode registers
//	// we need to save this as global variable to get the timings right
//	_bitMask = digitalPinToBitMask(pin);
//	uint8_t port = digitalPinToPort(pin);
//	_outPort = portOutputRegister(port);
//
//	// set pin to OUTPUT
//	volatile uint8_t * _modePort = portModeRegister(port);
//	*_modePort |= _bitMask;
//
//	// disable interrupts
//	//uint8_t oldSREG = SREG;
//	//cli();
//
//	const int repeat = 1;
//	for (int i = 0; i < repeat; i++)
//		writeNEC(pin, IRData);
//
//	// enable interrupts
//	//SREG = oldSREG;
//
//	// set pin to INPUT again to be save
//	*_modePort &= ~_bitMask;
//}


//void CIRLremote::writeNEC(const uint8_t pin, IR_Remote_Data_t IRData)
//{
//	// send header
//	mark38_4(NEC_MARK_LEAD);
//	if (IRData.command == 0xFFFF)
//		// space
//		space(NEC_SPACE_HOLDING);
//	else{
//		// normal signal
//		space(NEC_SPACE_LEAD);
//		for (int i = 0; i < (NEC_BLOCKS * 8); i++) {
//			// send logic bits
//			uint8_t index = i / 8;
//			if (IRData.whole[index] & 0x80) {
//				mark38_4(NEC_MARK_ZERO);
//				space(NEC_SPACE_ONE);
//			}
//			else {
//				mark38_4(NEC_MARK_ZERO);
//				space(NEC_SPACE_ZERO);
//			}
//			// get next bit
//			IRData.whole[index] <<= 1;
//		}
//	}
//	// finish mark
//	mark38_4(NEC_MARK_ZERO);
//	space(0);
//}
//
//void CIRLremote::mark38_4(int time) {
//	// Sends an IR mark for the specified number of microseconds.
//	// The mark output is modulated at the PWM frequency.
//	// 1/38.4kHz = 0.00002604166
//
//	while ((time -= 26) > 0){
//		*_outPort |= _bitMask;
//		delayMicroseconds(12);
//		*_outPort &= ~_bitMask;
//		delayMicroseconds(13);
//	}
//}
//
//void CIRLremote::mark37(int time) {
//	// Sends an IR mark for the specified number of microseconds.
//	// The mark output is modulated at the PWM frequency.
//	// 1/37kHz = 0.00002702702
//	while ((time -= 27) > 0){
//		*_outPort |= _bitMask;
//		delayMicroseconds(13);
//		*_outPort &= ~_bitMask;
//		delayMicroseconds(13);
//	}
//
//	//time = time / 26;
//	//while (time--){
//	//	*_outPort |= _bitMask;
//	//	delayMicroseconds(12);
//	//	*_outPort &= ~_bitMask;
//	//	delayMicroseconds(13);
//	//}
//}
//
//void CIRLremote::space(int time) {
//	// Sends an IR space for the specified number of microseconds.
//	// A space is no output, so the PWM output is disabled.
//	*_outPort &= ~_bitMask; // write pin LOW
//	delayMicroseconds(time);
//}
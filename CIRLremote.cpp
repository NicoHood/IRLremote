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

CIRLremote IRLremote(IR_NEC);

// points to the class itself later, needed for the wrapper
CIRLremote * CIRLremote::active_object = NULL;

//CIRLremote::CIRLremote(IRType type) :irType(type){
//	// ensure available() and paused() returns false
//	newInput = false;
//	//pauseIR = false;
//}

void CIRLremote::begin(uint8_t interrupt, void(*function)(IR_Remote_Data_t)){
	// check if we already set an Interrupt (by this or another class), do not set it twice!
	if (active_object != NULL) return;

	// select the class itself for the wrapper
	active_object = this;

	// save new values
	mInterrupt = interrupt;
	//IRprotocol = &protocol;
	user_onReceive = function;

	// TODO pullup? only for 1.5.7 with pin to interrupt define possible
	// however the IR receiver chips normally have built in pullups
	//pinMode(2, INPUT_PULLUP);

	// do whatever is necessary to reset the decode function
	reset();
	mLastTime = 0; // TODO move to constructor if getTimeout() is used

	// attach the wrapper function that calls our main function on an interrupt
	attachInterrupt(mInterrupt, interruptIR_wrapper, CHANGE);
}

void CIRLremote::end(void){
	// if someone calls end() without calling begin() first or calls end() twice
	if (active_object != this) return;

	// release the interrupt, if its NOT_AN_INTERRUPT the detach function does nothing
	detachInterrupt(mInterrupt);

	// ensure available(), paused() doesnt return anything
	newInput = false;
	//pauseIR = false;

	// deselect the class
	active_object = NULL;
}

bool CIRLremote::available(void){
	return newInput;
}

IR_Remote_Data_t CIRLremote::read(void){
	IR_Remote_Data_t IRReport;

	// reset remote and return data. Copy before unpausing!
	if (newInput){
		// save new value
		IRReport = IRData;

		// fires the protocols reset function
		//IRprotocol->reset();

		// resume reading
		newInput = false;
	}
	// return empty report
	else
		memset(&IRReport, 0, sizeof(IRReport));

	return IRReport;
}

//unsigned long CIRLremote::getTimeout(void){
//	// returns time since last signal
//	return micros() - mLastTime;
//}
//
//bool CIRLremote::paused(void){
//	return pauseIR;
//}
//
//void CIRLremote::pause(void){
//	pauseIR = true;
//}
//
//void CIRLremote::resume(void){
//	pauseIR = false;
//}
//
//void CIRLremote::reset(void){
//	// calls the protocol's reset function
//	IRprotocol->reset();
//
//	// resume reading
//	newInput = false;
//	pauseIR = false;
//}

void CIRLremote::interruptIR_wrapper(void){
	// called by interrupt CHANGE
	// call the instance. Needed for non static functions
	//if (active_object)
	active_object->interruptIR();
}

void CIRLremote::interruptIR(void){
	// for no user function set we need to pause IR
	// to not overwrite the actual values until they are read.
	if (newInput) return; //  || pauseIR

	//save the duration between the last reading
	unsigned long time = micros();
	unsigned long duration = time - mLastTime;
	mLastTime = time;

	// check if we have a new input and call the userfunction
	// if no userfunction is set, flag a newinput signal and block
	if (decodeIR(duration)){
		if (user_onReceive != NULL)
			user_onReceive(IRData);
		else
			newInput = true;
	}
}


void CIRLremote::write(const uint8_t pin, IR_Remote_Data_t IRData)
{
	// Get the port mask and the pointers to the out/mode registers
	// we need to save this as global variable to get the timings right
	_bitMask = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);
	_outPort = portOutputRegister(port);

	// set pin to OUTPUT
	volatile uint8_t * _modePort = portModeRegister(port);
	*_modePort |= _bitMask;

	// disable interrupts
	//uint8_t oldSREG = SREG;
	//cli();

	const int repeat = 1;
	for (int i = 0; i < repeat; i++)
		writeNEC(pin, IRData);

	// enable interrupts
	//SREG = oldSREG;

	// set pin to INPUT again to be save
	*_modePort &= ~_bitMask;
}


void CIRLremote::writeNEC(const uint8_t pin, IR_Remote_Data_t IRData)
{
	// send header
	mark38_4(NEC_MARK_LEAD);
	if (IRData.command == 0xFFFF)
		// space
		space(NEC_SPACE_HOLDING);
	else{
		// normal signal
		space(NEC_SPACE_LEAD);
		for (int i = 0; i < (NEC_BLOCKS * 8); i++) {
			// send logic bits
			uint8_t index = i / 8;
			if (IRData.whole[index] & 0x80) {
				mark38_4(NEC_MARK_ZERO);
				space(NEC_SPACE_ONE);
			}
			else {
				mark38_4(NEC_MARK_ZERO);
				space(NEC_SPACE_ZERO);
			}
			// get next bit
			IRData.whole[index] <<= 1;
		}
	}
	// finish mark
	mark38_4(NEC_MARK_ZERO);
	space(0);
}

void CIRLremote::mark38_4(int time) {
	// Sends an IR mark for the specified number of microseconds.
	// The mark output is modulated at the PWM frequency.
	// 1/38.4kHz = 0.00002604166

	while ((time -= 26) > 0){
		*_outPort |= _bitMask;
		delayMicroseconds(12);
		*_outPort &= ~_bitMask;
		delayMicroseconds(13);
	}
}

void CIRLremote::mark37(int time) {
	// Sends an IR mark for the specified number of microseconds.
	// The mark output is modulated at the PWM frequency.
	// 1/37kHz = 0.00002702702
	while ((time -= 27) > 0){
		*_outPort |= _bitMask;
		delayMicroseconds(13);
		*_outPort &= ~_bitMask;
		delayMicroseconds(13);
	}

	//time = time / 26;
	//while (time--){
	//	*_outPort |= _bitMask;
	//	delayMicroseconds(12);
	//	*_outPort &= ~_bitMask;
	//	delayMicroseconds(13);
	//}
}

void CIRLremote::space(int time) {
	// Sends an IR space for the specified number of microseconds.
	// A space is no output, so the PWM output is disabled.
	*_outPort &= ~_bitMask; // write pin LOW
	delayMicroseconds(time);
}
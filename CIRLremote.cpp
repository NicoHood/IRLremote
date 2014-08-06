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

CIRLremote IRLremote;

// points to the class itself later, needed for the wrapper
CIRLremote * CIRLremote::active_object = NULL;

CIRLremote::CIRLremote(void){
	// ensure available() doesnt return anything
	pauseIR = false;
}

void CIRLremote::begin(uint8_t interrupt, CIRLprotocol &protocol, void(*function)(IR_Remote_Data_t)){
	// check if we already set an Interrupt (by this or another class), do not set it twice!
	if (active_object != NULL) return;

	// select the class itself for the wrapper
	active_object = this;

	// reset variables
	pauseIR = false;
	mLastTime = 0;

	// save new values
	mInterrupt = interrupt;
	IRprotocol = &protocol;
	user_onReceive = function;

	// TODO pullup? only for 1.5.7 with pin to interrupt define possible
	// however the IR receiver chips normally have built in pullups
	//pinMode(2, INPUT_PULLUP);

	// do whatever is necessary to reset the decode function
	IRprotocol->reset();

	// attach the wrapper function that calls our main function on an interrupt
	attachInterrupt(mInterrupt, interruptIR_wrapper, CHANGE);
}

void CIRLremote::end(void){
	// if someone calls end() without calling begin() first or calls end() twice
	if (active_object != this) return;

	// release the interrupt, if its NOT_AN_INTERRUPT the detach function does nothing
	detachInterrupt(mInterrupt);

	// ensure available() doesnt return anything
	pauseIR = false;

	// deselect the class
	active_object = NULL;
}

bool CIRLremote::available(void){
	return pauseIR;
}

IR_Remote_Data_t CIRLremote::read(void){
	IR_Remote_Data_t IRReport;

	// reset remote and return data. Copy before unpausing!
	if (pauseIR){
		IRReport = IRprotocol->IRData;
		pauseIR = false;
	}
	// return empty report
	else
		memset(&IRReport, 0, sizeof(IRReport));

	return IRReport;
}

void CIRLremote::interruptIR_wrapper(void){ //called interrupt CHANGE
	// call the instance. Needed for non static functions
	if (active_object)
		active_object->interruptIR();
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
	if (IRprotocol->decodeIR(duration)){
		if (user_onReceive != NULL)
			user_onReceive(IRprotocol->IRData);
		else
			pauseIR = true;
	}
}
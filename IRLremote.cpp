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

#include <IRLremote.h>

//================================================================================
// User function helper
//================================================================================

// ensure available() returns false
uint8_t  IRLProtocol = 0;
uint16_t IRLAddress = 0;
uint32_t IRLCommand = 0;

// Called when directly received correct IR Signal
// Do not use Serial inside, it can crash your Arduino!
void IREvent(uint8_t protocol, uint16_t address, uint32_t command) {
	// For no user function set we need to pause IR
	// to not overwrite the actual values until they are read.
	if (IRLProtocol) return;
	IRLAddress = address;
	IRLCommand = command;
	IRLProtocol = protocol;
}

void decodeIR(const uint32_t duration){
	// add your own protocols here or create your own decoding function
	// you can overwrite this function in your .ino file
	decodeAll(duration);
}


//================================================================================
// Decoding
//================================================================================



//================================================================================
// Sending
//================================================================================

uint8_t _bitMask;
volatile uint8_t * _outPort;
void IRLwrite(const uint8_t pin, uint16_t address, uint32_t command)
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
		IRLwriteNEC(pin, address, command);

	// enable interrupts
	//SREG = oldSREG;

	// set pin to INPUT again to be save
	*_modePort &= ~_bitMask;
}


void IRLwriteNEC(const uint8_t pin, uint16_t address, uint32_t command)
{
	// send header
	mark38_4(NEC_MARK_LEAD);
	if (command == 0xFFFF)
		// space
		space(NEC_SPACE_HOLDING);
	else{
		// normal signal
		space(NEC_SPACE_LEAD);
		for (int i = 0; i < (NEC_BLOCKS * 8); i++) {
			// the bitorder is a mess i know.
			bool bit;
			if (i < (2 * 8))
				bit = ((address >> ((i / 8) * 8))&(0x80 >> (i % 8))) ? 1 : 0;
			else
				bit = ((command >> (((i / 8) - 2) * 8))&(0x80 >> (i % 8))) ? 1 : 0;

			// send logic bits
			mark38_4(NEC_MARK_ZERO);
			if (bit)
				space(NEC_SPACE_ONE);
			else
				space(NEC_SPACE_ZERO);
		}

		// finish mark
		mark38_4(NEC_MARK_ZERO);
		space(0);
	}
}

void mark38_4(int time) {
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

void mark37(int time) {
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

void space(int time) {
	// Sends an IR space for the specified number of microseconds.
	// A space is no output, so the PWM output is disabled.
	*_outPort &= ~_bitMask; // write pin LOW
	delayMicroseconds(time);
}
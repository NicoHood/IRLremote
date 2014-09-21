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

void IRLwrite(const uint8_t pin, uint16_t address, uint32_t command)
{
	// get the port mask and the pointers to the out/mode registers
	uint8_t bitMask = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);
	volatile uint8_t * outPort = portOutputRegister(port);
	volatile uint8_t * modePort = portModeRegister(port);

	// set pin to OUTPUT and LOW
	*modePort |= bitMask;
	*outPort &= ~bitMask;

	// disable interrupts
	//uint8_t oldSREG = SREG;
	//cli();

	const int repeat = 1;
	for (int i = 0; i < repeat; i++)
		IRLwriteNEC(outPort, bitMask, address, command);

	// enable interrupts
	//SREG = oldSREG;

	// set pin to INPUT again to be save
	*modePort &= ~bitMask;
}


void IRLwriteNEC(volatile uint8_t * outPort, uint8_t bitmask, uint16_t address, uint32_t command)
{
	// send header
	mark(NEC_HZ, outPort, bitmask, NEC_MARK_LEAD);
	if (command == 0xFFFF)
		// space
		space(outPort, bitmask, NEC_SPACE_HOLDING);
	else{
		// normal signal
		space(outPort, bitmask, NEC_SPACE_LEAD);
		for (int i = 0; i < (NEC_BLOCKS * 8); i++) {
			// the bitorder is a mess i know.
			bool bit;
			if (i < (2 * 8))
				bit = ((address >> ((i / 8) * 8))&(0x80 >> (i % 8))) ? 1 : 0;
			else
				bit = ((command >> (((i / 8) - 2) * 8))&(0x80 >> (i % 8))) ? 1 : 0;

			// send logic bits
			mark(NEC_HZ, outPort, bitmask, NEC_MARK_ZERO);
			if (bit)
				space(outPort, bitmask, NEC_SPACE_ONE);
			else
				space(outPort, bitmask, NEC_SPACE_ZERO);
		}

		// finish mark
		mark(NEC_HZ, outPort, bitmask, NEC_MARK_ZERO);
		space(outPort, bitmask, 0);
	}
}

void mark(const uint16_t Hz, volatile uint8_t * outPort, uint8_t bitMask, uint16_t time) {
	/*
	Bitbangs PWM in the given Hz number for the given time
	________________________________________________________________________________
	Delay calculation:
	F_CPU/1.000.000 to get number of cycles/uS
	/3 to get the number of loops needed for 1ms (1loop = 3 cycles)

	Multiply with the number of ms delay:
	1/kHz to get the seconds
	* 1.000.000 to get it in uS
	/2 to get half of a full pulse

	Substract the while, portmanipulation, loop overhead /3 loop cycles

	F_CPU(16.000.000)            1 * 1.000.000(pulse in ms)   12(overhead)
	========================== * ========================== - ==============
	1.000.000 * 3(loop cycles)   Hz * 2(half of a pulse)      3(loop cycles)

	<==>

	F_CPU(16.000.000) - (12(overhead) * Hz * 2(half of a pulse))
	===========================================================
	Hz * 2(half of a on/off pulse) * 3(loop cycles)

	________________________________________________________________________________
	Iterations calculation:
	Devide time with cycles in while loop
	Multiply this with the cycles per uS
	cycles per while loop: 3(loop cycles) * delay + overhead

	time * (F_CPU(16.000.000) / 1.000.000)
	======================================
	delay*3(loop cycles) + overhead
	*/

	const uint32_t loopCycles = 3;
	const uint32_t overHead = 12;
	uint8_t delay = (F_CPU - (overHead * Hz * 2UL)) / (Hz * 2UL * loopCycles);
	uint16_t iterations = (time*(F_CPU / 1000000UL)) / (delay * loopCycles + overHead);

	while (iterations--){
		*outPort ^= bitMask;
		_delay_loop_1(delay);
	}
}

void space(volatile uint8_t * outPort, uint8_t bitMask, uint16_t time) {
	// Sends an IR space for the specified number of microseconds.
	// A space is no output, so the PWM output is disabled.
	*outPort &= ~bitMask; // write pin LOW
	delayMicroseconds(time);
	//_delay_loop_2(time*(F_CPU/1000000UL)/4UL);
}
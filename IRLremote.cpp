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
uint16_t IRLAddress = 0;
uint32_t IRLCommand = 0;
uint8_t  IRLProtocol = false;

// Called when directly received correct IR Signal
// Do not use Serial inside, it can crash your Arduino!
void irEvent(uint8_t protocol, uint16_t address, uint32_t command) {
	// For no user function set we need to pause IR
	// to not overwrite the actual values until they are read.
	if (IRLProtocol) return;
	IRLAddress = address;
	IRLCommand = command;
	IRLProtocol = protocol;
}

//================================================================================
// Decoding
//================================================================================

void IRLinterrupt(void){
	//save the duration between the last reading
	static unsigned long lastTime = 0;
	unsigned long time = micros();
	unsigned long duration = time - lastTime;
	lastTime = time;

	//TODO
	const IRType irType = IR_ALL;
	//const IRType irType = IR_NEC;
	//const IRType irType = IR_PANASONIC;

	// determinate which decode function must be called
	switch (irType){
	case IR_ALL:
		decodeNEC(duration);
		decodePanasonic(duration);
		break;
	case IR_NEC:
		decodeNEC(duration);
		break;
	case IR_PANASONIC:
		decodePanasonic(duration);
		break;
	}
}

void decodeNEC(const uint32_t duration){
	// temporary buffer to hold bytes for decoding this protocol
	static uint8_t data[NEC_BLOCKS];
	// pass the duration to the decoding function
	if (IRLdecodeSpace <NEC_TIMEOUT, NEC_MARK_LEAD, NEC_SPACE_LEAD, NEC_SPACE_HOLDING,
		NEC_SPACE_ZERO, NEC_SPACE_ONE, NEC_LENGTH, NEC_BLOCKS>
		(duration, data)){
		// Check if the protcol's checksum is correct
		// In some other Nec Protocols the Address has an inverse or not, so we only check the command
		if (IRLcheckInverse1(data) || IRLcheckHolding(data)){
			// you could check the address for inverse0 too
			// but newer devices use an extended address without inverse
			uint16_t address = (data[0] << 8) | data[1];
			uint32_t command = ((data[2] << 8) | data[3]) & 0xFFFF;
			irEvent(IR_NEC, address, command);
			return;
		}
	}
}

void decodePanasonic(const uint32_t duration){
	// temporary buffer to hold bytes for decoding this protocol
	static uint8_t data[PANASONIC_BLOCKS];
	// pass the duration to the decoding function
	if (IRLdecodeSpace <PANASONIC_TIMEOUT, PANASONIC_MARK_LEAD, PANASONIC_SPACE_LEAD, PANASONIC_SPACE_HOLDING,
		PANASONIC_SPACE_ZERO, PANASONIC_SPACE_ONE, PANASONIC_LENGTH, PANASONIC_BLOCKS>
		(duration, data)){
		// Check if the protcol's checksum is correct
		if (IRLcheckXOR0(data)){
			uint16_t address = (data[0] << 8) | data[1];
			uint32_t command = (uint32_t(data[2]) << 24) | (uint32_t(data[3]) << 16) | (uint32_t(data[4]) << 8) | data[5];
			irEvent(IR_PANASONIC, address, command);
			return;
		}
	}
}

template <uint32_t timeout, uint16_t markLead, uint16_t spaceLead, uint16_t spaceHolding,
	uint16_t spaceZero, uint16_t spaceOne, uint16_t irLength, uint8_t blocks>
	bool IRLdecodeSpace(unsigned long duration, uint8_t data[]){
	// variables for ir processing

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
				// set command to 0xFF if button is held down
				if (blocks <= 4){
					data[0] = data[1] = 0x00;
					data[2] = data[3] = 0xFF;
				}
				count = 0;
				return true;
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
	// TODO calculate with blocks to not go over bounds
	if (count >= irLength){
		count = 0;
		return true;
	}
	return false;
}

//================================================================================
// Sending
//================================================================================


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
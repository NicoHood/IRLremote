/*
Copyright (c) 2014-2015 NicoHood
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

// include guard
#pragma once

//================================================================================
// Definitions
//================================================================================

// definition to convert an uint8_t array to an uint16_t/uint32_t at any position (thx timeage!)
#define UINT16_AT_OFFSET(p_to_8, offset)    ((uint16_t)*((const uint16_t *)((p_to_8)+(offset))))
#define UINT32_AT_OFFSET(p_to_8, offset)    ((uint32_t)*((const uint32_t *)((p_to_8)+(offset))))

//================================================================================
// User Functions
//================================================================================

template <uint32_t debounce, IRType ...irProtocol>
inline CIRLremote<debounce, irProtocol...>::
CIRLremote(void){
	// empty
}


template <uint32_t debounce, IRType ...irProtocol>
inline bool CIRLremote<debounce, irProtocol...>::
begin(uint8_t pin){
	// try to attach PinInterrupt first
	if (digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT)
		attachInterrupt(digitalPinToInterrupt(pin), IRLinterrupt, CHANGE);

	// if PCINT library used, try to attach it
#ifdef PCINT_VERSION
	else if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT)
		attachPCINT(digitalPinToPCINT(pin), IRLinterrupt, CHANGE);
#endif

	// return an error if none of them work
	else
		return false;

	// if it passes the attach function everything went okay.
	return true;
}


template <uint32_t debounce, IRType ...irProtocol>
inline bool CIRLremote<debounce, irProtocol...>::
end(uint8_t pin){
	// try to detach PinInterrupt first
	if (digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT)
		detachInterrupt(digitalPinToInterrupt(pin));

	// if PCINT library used, try to detach it
#ifdef PCINT_VERSION
	else if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT)
		detachPCINT(digitalPinToPCINT(pin));
#endif

	// return an error if none of them work
	else
		return false;

	// if it passes the detach function everything went okay.
	return true;
}


template <uint32_t debounce, IRType ...irProtocol>
inline bool CIRLremote<debounce, irProtocol...>::
available(void){
	// TODO if more sony protocols are supported give the protocol some more time to finish or abort
	//if (micros() - lastEvent > (SONY_MARK_ZERO + SONY_MARK_ONE))
	return ((protocol > 0) ? true : false);
	//else
	//	return false;
}


template <uint32_t debounce, IRType ...irProtocol>
inline uint8_t CIRLremote<debounce, irProtocol...>::
getProtocol(void){
	return protocol;
}


template <uint32_t debounce, IRType ...irProtocol>
inline uint16_t CIRLremote<debounce, irProtocol...>::
getAddress(void){
	// value will be kept after a reset() for button holding
	// protocol check removed due to more overhead
	//if (protocol)
	return address;
	//else
	//	return 0;
}


template <uint32_t debounce, IRType ...irProtocol>
inline uint32_t CIRLremote<debounce, irProtocol...>::
getCommand(void){
	// value will be kept after a reset() for button holding
	// protocol check removed due to more overhead
	//if (protocol)
	return command;
	//else
	//	return 0;
}


template <uint32_t debounce, IRType ...irProtocol>
inline void CIRLremote<debounce, irProtocol...>::
reset(void){
	// the other values are kept after reading for button holding
	protocol = 0;
}


template <uint32_t debounce, IRType ...irProtocol>
inline void CIRLremote<debounce, irProtocol...>::
IRLinterrupt(void){
	// function called by the interrupt
	//save the duration between the last reading
	uint32_t time = micros();
	uint32_t duration_32 = time - lastTime;
	lastTime = time;

	// calculate 16 bit duration. On overflow sets duration to a clear timeout
	uint16_t duration = 0xFFFF;
	if (duration_32 <= 0xFFFF)
		duration = duration_32;

	// go through all known protocols and decode if they are selected
	// check if the user specified the protocols
	// reorder the protocols to get the optimal sketch size
	if (protocolAvailable<IR_SONY12>())
		decodeSony12(duration);
	if (protocolAvailable<IR_PANASONIC>())
		decodePanasonic(duration);
	if (protocolAvailable<IR_NEC>())
		decodeNec(duration);
	if (protocolAvailable<IR_SONY20>())
		decodeSony20(duration);
}


//================================================================================
// Event Functions
//================================================================================

template <uint32_t debounce, IRType ...irProtocol>
inline void CIRLremote<debounce, irProtocol...>::
IREvent(uint8_t p, uint16_t a, uint32_t c) {
	// called when directly received a valid IR signal.
	// do not use Serial inside, it can crash your program!

	// dont update value if we already have a signal
	if (protocol){
		// check if we have a longer sony signal
		// TODO also check if sony is selected via the template, only then do this compare
		//if (protocol == IR_SONY12 && p == IR_SONY15)
		//	/* continue */;
		//else
		return;
	}

	// check if the command is the same and if the last signal was received too fast
	// do not save the new time, to not block forever if the user is holding a button
	// this way you can still realize things like: hold a button to increase the volume
	if (/*(command == c) && */((lastTime - lastEvent) < (debounce * 1000UL)))
		return;

	// update values
	lastEvent = lastTime;

	// update the values to the newest valid input
	protocol = p;
	address = a;
	command = c;
}


template <uint32_t debounce, IRType ...irProtocol>
template <IRType ir>
inline void CIRLremote<debounce, irProtocol...>::
buttonHolding(void){
	// determine timeout value
	uint32_t timeout = 0;
	if (ir == IR_NEC)
		timeout = NEC_TIMEOUT_REPEAT;

	// check if time is greater debounce value or smaller debounce + the max timespan of a repeat signal
	// this prevents to trigger after a few seconds if the first press was lost
	if (((lastTime - lastEvent) < (debounce * 1000UL)) || ((lastTime - lastEvent) >= (debounce * 1000UL + timeout)))
		return;

	// update values
	lastEvent = lastTime;

	// update the values to the last valid input (command + address are still saved and not overwritten)
	protocol = ir;
}


//================================================================================
// Decode Functions
//================================================================================

template <uint32_t debounce, IRType ...irProtocol>
template <IRType ir>
inline bool CIRLremote<debounce, irProtocol...>::
protocolAvailable(void) {
	if (sizeof...(irProtocol) == 0)
		return true;

	// unroll all used protocols in array and check if its the needed one
	bool inArray[]{
		((irProtocol == ir) ? true : false)...
	};

	// go through all protocols and check if the needed one is there
	for (uint8_t i = 0; i < sizeof...(irProtocol); i++)
		if (inArray[i] == true)
			return true;
	return false;
}


template <uint32_t debounce, IRType ...irProtocol>
inline void CIRLremote<debounce, irProtocol...>::
decodeNec(const uint16_t duration){
	// pass the duration to the decoding function
	bool newInput;
	// no accuracy set at the moment, no conflict detected yet
	// probably due to the checksum
	//if (sizeof...(irProtocol) != 1)
	newInput = IRLdecode <NEC_LENGTH, (NEC_TIMEOUT + NEC_MARK_LEAD) / 2, // irLength, timeoutThreshold
		(NEC_MARK_LEAD + NEC_SPACE_ONE) / 2, (NEC_SPACE_LEAD + NEC_SPACE_HOLDING) / 2, // markLeadThreshold, spaceLeadThreshold
		(NEC_SPACE_HOLDING + NEC_SPACE_ONE) / 2, 0, // spaceLeadHoldingThreshold, markThreshold
		(NEC_SPACE_ONE + NEC_SPACE_ZERO) / 2, // spaceThreshold
		0, 0>// markTimeout, spaceTimeout
		(duration, dataNec, countNec);
	//else

	if (newInput){
		// check for button holding
		if (countNec == 2)
			buttonHolding<IR_NEC>();

		// Check if the protcol's checksum is correct
		// check if byte 0 and is the inverse of byte 1
		else if (uint8_t((dataNec[2] ^ (~dataNec[3]))) == 0x00){
			// normally NEC also check for the inverse of the address.
			// newer remotes dont have this because of the wide used protocol all addresses were already used
			// to make it less complicated its left out and the user can check the command inverse himself if needed
			// check if byte 2 and is the inverse of byte 3
			//if (uint8_t((data[0] ^ (~data[1]))) != 0x00)
			//	return;

			// save address + command and trigger event
			uint16_t address = UINT16_AT_OFFSET(dataNec, 0);
			uint32_t command = UINT16_AT_OFFSET(dataNec, 2);
			IREvent(IR_NEC, address, command);
		}

		// reset reading
		countNec = 0;
	}
}


template <uint32_t debounce, IRType ...irProtocol>
inline void CIRLremote<debounce, irProtocol...>::
decodePanasonic(const uint16_t duration){
	// pass the duration to the decoding function
	bool newInput;
	//if (sizeof...(irProtocol) != 1) // no accuracy set at the moment, no conflict detected yet
	newInput = IRLdecode <PANASONIC_LENGTH, (PANASONIC_TIMEOUT + PANASONIC_MARK_LEAD) / 2, // irLength, timeoutThreshold
		(PANASONIC_MARK_LEAD + PANASONIC_SPACE_ONE) / 2, (PANASONIC_SPACE_LEAD + PANASONIC_SPACE_ONE) / 2, // markLeadThreshold, spaceLeadThreshold
		0, 0, // spaceLeadHoldingThreshold, markThreshold
		(PANASONIC_SPACE_ONE + PANASONIC_SPACE_ZERO) / 2, // spaceThreshold
		0, 0>// markTimeout, spaceTimeout
		(duration, dataPanasonic, countPanasonic);
	//else

	if (newInput){
		// Check if the protcol's checksum is correct
		if (uint8_t(dataPanasonic[2] ^ dataPanasonic[3] ^ dataPanasonic[4]) == dataPanasonic[5]){
			// check vendor parity nibble (optional)
			//uint8_t XOR = data[0] ^ data[1];
			//if ((XOR & 0x0F ^ (XOR >> 4)) != 0x00)
			//	return;

			// address represents vendor(16)
			uint16_t address = UINT16_AT_OFFSET(dataPanasonic, 0);

			// command represents (MSB to LSB):
			// vendor parity(4), genre1(4), genre2(4), data(10), ID(2), parity(8)
			uint32_t command = UINT32_AT_OFFSET(dataPanasonic, 2);			IREvent(IR_PANASONIC, address, command);
			return;
		}

		// reset reading
		countPanasonic = 0;
	}
}


template <uint32_t debounce, IRType ...irProtocol>
inline void CIRLremote<debounce, irProtocol...>::
decodeSony12(const uint16_t duration){
	// pass the duration to the decoding function
	bool newInput;
	// 1st extra accuracy solution
	if (sizeof...(irProtocol) != 1)
		newInput = IRLdecode <SONY_LENGTH_12, (SONY_TIMEOUT + SONY_MARK_LEAD) / 2, // irLength, timeoutThreshold
		(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, 0, // markLeadThreshold, spaceLeadThreshold
		0, (SONY_MARK_ONE + SONY_MARK_ZERO) / 2, // spaceLeadHoldingThreshold, markThreshold
		0, // spaceThreshold
		(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, SONY_MARK_ONE>// markTimeout, spaceTimeout
		(duration, dataSony12, countSony12);
	else
		newInput = IRLdecode <SONY_LENGTH_12, (SONY_TIMEOUT + SONY_MARK_LEAD) / 2, // irLength, timeoutThreshold
		(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, 0, // markLeadThreshold, spaceLeadThreshold
		0, (SONY_MARK_ONE + SONY_MARK_ZERO) / 2, // spaceLeadHoldingThreshold, markThreshold
		0, // spaceThreshold
		0, 0>// markTimeout, spaceTimeout
		(duration, dataSony12, countSony12);

	if (newInput){
		// protocol has no checksum
		uint8_t upper4Bits = ((dataSony12[1] >> 3) & 0x1E);
		if (dataSony12[0] & 0x80)
			upper4Bits |= 0x01;
		else
			upper4Bits &= ~0x01;
		uint16_t address = upper4Bits;
		uint32_t command = dataSony12[0] & 0x7F;
		// 2nd extra accuracy solution
		//if ((sizeof...(irProtocol) != 1) && (address || command))
		IREvent(IR_SONY12, address, command);

		// reset reading
		countSony12 = 0;
	}
}


template <uint32_t debounce, IRType ...irProtocol>
inline void CIRLremote<debounce, irProtocol...>::
decodeSony20(const uint16_t duration){
	// pass the duration to the decoding function
	bool newInput;
	// 1st extra accuracy solution
	if (sizeof...(irProtocol) != 1)
		newInput = IRLdecode <SONY_LENGTH_20, (SONY_TIMEOUT + SONY_MARK_LEAD) / 2, // irLength, timeoutThreshold
		(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, 0, // markLeadThreshold, spaceLeadThreshold
		0, (SONY_MARK_ONE + SONY_MARK_ZERO) / 2, // spaceLeadHoldingThreshold, markThreshold
		0, // spaceThreshold
		(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, SONY_MARK_ONE>// markTimeout, spaceTimeout
		(duration, dataSony20, countSony20);
	else
		newInput = IRLdecode <SONY_LENGTH_20, (SONY_TIMEOUT + SONY_MARK_LEAD) / 2, // irLength, timeoutThreshold
		(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, 0, // markLeadThreshold, spaceLeadThreshold
		0, (SONY_MARK_ONE + SONY_MARK_ZERO) / 2, // spaceLeadHoldingThreshold, markThreshold
		0, // spaceThreshold
		0, 0>// markTimeout, spaceTimeout
		(duration, dataSony20, countSony20);

	if (newInput){
		// protocol has no checksum
		uint8_t upper5Bits = ((dataSony20[2] >> 2) & 0x3E);
		uint8_t lsb = (dataSony20[0] >> 7) & 0x01;
		uint16_t address = (upper5Bits << 8) | (dataSony20[1] << 1) | lsb;
		uint32_t command = dataSony20[0] & 0x7F;
		// 2nd extra accuracy solution
		//if ((sizeof...(irProtocol) != 1) && (address || command))
		IREvent(IR_SONY20, address, command);

		// reset reading
		countSony20 = 0;
	}
}


template <uint32_t debounce, IRType ...irProtocol>
template <uint8_t irLength, uint16_t timeoutThreshold, uint16_t markLeadThreshold, uint16_t spaceLeadThreshold,
	uint16_t spaceLeadHoldingThreshold, uint16_t markThreshold, uint16_t spaceThreshold,
	uint16_t markTimeout, uint16_t spaceTimeout>
	inline bool CIRLremote<debounce, irProtocol...>::
	IRLdecode(uint16_t duration, uint8_t data[], uint8_t &count){

	// if timeout always start next possible reading and abort any pending readings
	if (duration >= timeoutThreshold)
		count = 1;

	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
	// this is to not conflict with other protocols while they are sending 0/1 which might be similar to a lead in another protocol
	else if (count == 0)
		return false;

	// check pulses for mark/space and lead + logical 0/1 seperate
	else{
		// Mark pulses (odd numbers)
		if (count % 2 == 1){
			// check Mark Lead (needs a timeout or a correct signal)
			if (markLeadThreshold && count == 1){
				// lead is okay// wrong lead
				if (duration <= markLeadThreshold){
					count = 0;
					return false;
				}
			}

			else{
				// check for timeout if needed (might be a different protocol)
				if (markTimeout && duration > markTimeout){
					count = 0;
					return false;
				}

				// only check values if the protocol has different logical space pulses
				else if (markThreshold){

					// get number of the Mark Bits (starting from zero)
					uint8_t length;
					// only save every 2nd value, substract the first two lead pulses
					if (!spaceThreshold)
						length = (count / 2) - 1;
					// special case: spaces and marks both have data in the pulse
					else length = count - 2;

					// move bits and write 1 or 0 depending on the duration
					// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
					data[length / 8] >>= 1;
					if (duration > markThreshold)
						data[length / 8] |= 0x80;
					//else // normally not needed through the bitshift
					//	data[length / 8] &= ~0x80;
				}

				// check last input (always a mark)
				if (count > irLength){
					// reset by decoding function
					//count = 0;
					return true;
				}
			}
		}

		// Space pulses (even numbers)
		else{
			//check Space Lead/Space Holding
			if (spaceLeadThreshold && count == 2){
				// normal Space, next reading
				if (duration > spaceLeadThreshold);

				// Button holding (if supported by protocol)
				else if (spaceLeadHoldingThreshold && duration > spaceLeadHoldingThreshold){
					// call the holding function after
					// count not reseted to read it afterwards
					//count = 0;
					return true;
				}
				// wrong space
				else {
					count = 0;
					return false;
				}
			}
			else{
				// check for timeout if needed (might be a different protocol)
				if (spaceTimeout && duration > spaceTimeout){
					count = 0;
					return false;
				}

				// only check values if the protocol has different logical space pulses
				else if (spaceThreshold){

					// get number of the Space Bits (starting from zero)
					uint8_t length;
					// only save every 2nd value, substract the first two lead pulses
					if (!markThreshold)
						length = (count / 2) - 2;
					// special case: spaces and marks both have data in the pulse
					else length = count - 2;

					// move bits and write 1 or 0 depending on the duration
					// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
					data[length / 8] >>= 1;
					if (duration > spaceThreshold)
						data[length / 8] |= 0x80;
					//else // normally not needed through the bitshift
					//	data[length / 8] &= ~0x80;
				}
			}
		}

		// next reading, no errors
		count++;
	}

	// no valid input (yet)
	return false;
}


//================================================================================
// Variables
//================================================================================

template <uint32_t debounce, IRType ...irProtocol>
uint32_t CIRLremote<debounce, irProtocol...>::
lastTime = 0;

template <uint32_t debounce, IRType ...irProtocol>
uint32_t CIRLremote<debounce, irProtocol...>::
lastEvent = 0;

template <uint32_t debounce, IRType ...irProtocol>
uint8_t CIRLremote<debounce, irProtocol...>::
protocol = 0;

template <uint32_t debounce, IRType ...irProtocol>
uint16_t CIRLremote<debounce, irProtocol...>::
address = 0;

template <uint32_t debounce, IRType ...irProtocol>
uint32_t CIRLremote<debounce, irProtocol...>::
command = 0;

template <uint32_t debounce, IRType ...irProtocol>
uint8_t CIRLremote<debounce, irProtocol...>::
countNec = 0;

template <uint32_t debounce, IRType ...irProtocol>
uint8_t CIRLremote<debounce, irProtocol...>::
dataNec[NEC_BLOCKS] = { 0 };

template <uint32_t debounce, IRType ...irProtocol>
uint8_t CIRLremote<debounce, irProtocol...>::
countPanasonic = 0;

template <uint32_t debounce, IRType ...irProtocol>
uint8_t CIRLremote<debounce, irProtocol...>::
dataPanasonic[PANASONIC_BLOCKS] = { 0 };

template <uint32_t debounce, IRType ...irProtocol>
uint8_t CIRLremote<debounce, irProtocol...>::
countSony12 = 0;

template <uint32_t debounce, IRType ...irProtocol>
uint8_t CIRLremote<debounce, irProtocol...>::
dataSony12[SONY_BLOCKS_12] = { 0 };

template <uint32_t debounce, IRType ...irProtocol>
uint8_t CIRLremote<debounce, irProtocol...>::
countSony20 = 0;

template <uint32_t debounce, IRType ...irProtocol>
uint8_t CIRLremote<debounce, irProtocol...>::
dataSony20[SONY_BLOCKS_20] = { 0 };

template <uint32_t debounce, IRType ...irProtocol>
uint32_t CIRLremote<debounce, irProtocol...>::
k[30] = { 0 };
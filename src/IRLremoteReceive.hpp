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
// User Functions
//================================================================================

template<uint32_t debounce, typename ...protocols>
CIRLremote<debounce, protocols...>::
CIRLremote(void) {
	// Empty
}


template<uint32_t debounce, typename ...protocols>
bool CIRLremote<debounce, protocols...>::
begin(uint8_t pin)
{
	// For single protocols use a different flag
	uint8_t flag = CHANGE;
	if(sizeof...(protocols) == 1){
		uint8_t flags [] = { protocols::getSingleFlag()... };
		flag = flags[0];
	}

	// Try to attach PinInterrupt first
	if (digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT){
		attachInterrupt(digitalPinToInterrupt(pin), interrupt, flag);
		return true;
	}

	// If PinChangeInterrupt library is used, try to attach it
#ifdef PCINT_VERSION
	else if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT){
		attachPCINT(digitalPinToPCINT(pin), interrupt, flag);
		return true;
	}
#endif

	// Return an error if none of them work (pin has no Pin(Change)Interrupt)
	return false;
}


template<uint32_t debounce, typename ...protocols>
bool CIRLremote<debounce, protocols...>::
end(uint8_t pin)
{
	// Try to detach PinInterrupt first
	if (digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT){
		detachInterrupt(digitalPinToInterrupt(pin));
		return true;
	}

	// If PinChangeInterrupt library is used, try to detach it
#ifdef PCINT_VERSION
	else if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT){
		detachPCINT(digitalPinToPCINT(pin));
		return true;
	}
#endif

	// Return an error if none of them work (pin has no Pin(Change)Interrupt)
	return false;
}


template<uint32_t debounce, typename ...protocols>
bool CIRLremote<debounce, protocols...>::
available(void)
{
	// This if construct saves flash
	if(CIRLData::IRLProtocol & IR_NEW_PROTOCOL)
		return true;
	else 
		return false;
}


template<uint32_t debounce, typename ...protocols>
IR_data_t CIRLremote<debounce, protocols...>::
read(void)
{
	// If nothing was received return an empty struct
	IR_data_t data = { 0 };
	
	// Only the received protocol will write data into the struct
	nop((protocols::read(&data), 0)...);
	
	// Return the new protocol information to the user
	return data;
}


//================================================================================
// Interrupt Function
//================================================================================

template<uint32_t debounce, typename ...protocols>
void CIRLremote<debounce, protocols...>::
interrupt(void) 
{ 
	// Block if the protocol is already recognized
	if (available())
		return;

	// Save the duration between the last reading
	uint32_t time = micros();
	uint32_t duration_32 = time - CIRLData::IRLLastTime;
	CIRLData::IRLLastTime = time;

	// Calculate 16 bit duration. On overflow sets duration to a clear timeout
	uint16_t duration = 0xFFFF;
	if (duration_32 <= 0xFFFF)
		duration = duration_32;
	
	// Call the decoding functions(s).
	if(sizeof...(protocols) == 1){
		// For a single protocol use a simpler decode function
		// to get maximum speed + recognition and minimum flash size
		nop((protocols::decodeSingle(duration, debounce), 0)...);
		//nop((protocols::template decodeSingle2<debounce>(duration), 0)...);
	}
	else{
		// Try to call all protocols decode functions
		nop((protocols::decode(duration), 0)...);
	}
	
	// Check if the last signal was received too fast.
	if (debounce && available()) 
	{
		// Do not save the new time, to not block forever if the user is holding a button.
		// This way you can still realize things like: hold a button to increase the volume
		if ((CIRLData::IRLLastTime - CIRLData::IRLLastEvent) < (debounce * 1000UL)){
			// Last input received too fast, ignore this one
			CIRLData::IRLProtocol &= ~IR_NEW_PROTOCOL;
			return;
		}

		// New valid signal, save new time
		CIRLData::IRLLastEvent = CIRLData::IRLLastTime;
	}
}



/*
template <uint32_t debounce, IRType ...irProtocol>
inline uint8_t CIRLremote<debounce, irProtocol...>::
decodePanasonicOnly(const uint16_t duration) {
	// no accuracy set at the moment, no conflict detected yet
	// due to the checksum we got a good recognition
	const uint8_t irLength = PANASONIC_LENGTH / 2;
	const uint16_t timeoutThreshold = (PANASONIC_TIMEOUT + PANASONIC_MARK_LEAD + PANASONIC_SPACE_LEAD) / 2;
	const uint16_t leadThreshold = (PANASONIC_MARK_LEAD + PANASONIC_SPACE_LEAD + PANASONIC_MARK_ONE + PANASONIC_SPACE_ONE) / 2;
	const uint16_t threshold = (PANASONIC_MARK_ONE + PANASONIC_MARK_ZERO + PANASONIC_SPACE_ONE + PANASONIC_SPACE_ZERO) / 2;

	// if timeout always start next possible reading and abort any pending readings
	if (duration >= timeoutThreshold)
		countPanasonic = 0;

	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
	// this is to not conflict with other protocols while they are sending 0/1
	// which might be similar to a lead in this protocol
	else if (countPanasonic == 0)
		return 0;

	// check Mark Lead (needs a timeout or a correct signal)
	else if (countPanasonic == 1) {
		// wrong lead
		if (duration < leadThreshold) {
			countPanasonic = 0;
			return 0;
		}
		// else normal lead, next reading
	}

	// pulses (mark + space)
	else {
		// check different logical space pulses

		// get number of the Space Bits (starting from zero)
		// only save every 2nd value, substract the first two lead pulses
		uint8_t length = countPanasonic - 2;

		// move bits and write 1 or 0 depending on the duration
		// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
		dataPanasonic[length / 8] >>= 1;
		// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
		if (duration >= threshold)
			dataPanasonic[length / 8] |= 0x80;


		// last mark (stop bit)
		if (countPanasonic >= irLength) {
			// reset reading
			countPanasonic = 0;

			// Check if the protcol's checksum is correct
			if (uint8_t(dataPanasonic[2] ^ dataPanasonic[3] ^ dataPanasonic[4]) == dataPanasonic[5])
				// check vendor parity nibble (optional)
				//uint8_t XOR = data[0] ^ data[1];
				//if ((XOR & 0x0F ^ (XOR >> 4)) != 0x00)
				//	return 0;
				// new input, now check for debounce
				return IR_PANASONIC;

			// checksum incorrect
			else
				return 0;
		}
	}

	// next reading, no errors
	countPanasonic++;
}


template <uint32_t debounce, IRType ...irProtocol>
inline uint8_t CIRLremote<debounce, irProtocol...>::
decodePanasonic(const uint16_t duration) {
	// no accuracy set at the moment, no conflict detected yet
	// due to the checksum we got a good recognition
	const uint8_t irLength = PANASONIC_LENGTH;
	const uint16_t timeoutThreshold = (PANASONIC_TIMEOUT + PANASONIC_MARK_LEAD) / 2;
	const uint16_t markLeadThreshold = (PANASONIC_MARK_LEAD + PANASONIC_SPACE_ONE) / 2;
	const uint16_t spaceLeadThreshold = (PANASONIC_SPACE_LEAD + PANASONIC_SPACE_ONE) / 2;
	const uint16_t spaceThreshold = (PANASONIC_SPACE_ONE + PANASONIC_SPACE_ZERO) / 2;

	// if timeout always start next possible reading and abort any pending readings
	if (duration >= timeoutThreshold)
		countPanasonic = 0;

	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
	// this is to not conflict with other protocols while they are sending 0/1
	// which might be similar to a lead in this protocol
	else if (countPanasonic == 0)
		return 0;

	// check Mark Lead (needs a timeout or a correct signal)
	else if (countPanasonic == 1) {
		// wrong lead
		if (duration < markLeadThreshold) {
			countPanasonic = 0;
			return 0;
		}
	}

	//check Space Lead/Space Holding
	else if (countPanasonic == 2) {
		// wrong space
		if (duration < spaceLeadThreshold) {
			countPanasonic = 0;
			return 0;
		}
		// else normal Space, next reading
	}

	// last mark (stop bit)
	else if (countPanasonic > irLength) {
		// reset reading
		countPanasonic = 0;

		// Check if the protcol's checksum is correct
		if (uint8_t(dataPanasonic[2] ^ dataPanasonic[3] ^ dataPanasonic[4]) == dataPanasonic[5])
			// check vendor parity nibble (optional)
			//uint8_t XOR = data[0] ^ data[1];
			//if ((XOR & 0x0F ^ (XOR >> 4)) != 0x00)
			//	return 0;
			// new input, now check for debounce
			return IR_PANASONIC;

		// checksum incorrect
		else
			return 0;
	}

	// Space pulses (even numbers)
	else if (countPanasonic % 2 == 0) {
		// check different logical space pulses

		// get number of the Space Bits (starting from zero)
		// only save every 2nd value, substract the first two lead pulses
		uint8_t length = (countPanasonic / 2) - 2;

		// move bits and write 1 or 0 depending on the duration
		// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
		dataPanasonic[length / 8] >>= 1;
		// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
		if (duration >= spaceThreshold)
			dataPanasonic[length / 8] |= 0x80;
	}
	// Mark pulses (odd numbers)
	// else ignored, always the same mark length

	// next reading, no errors
	countPanasonic++;
}


template <uint32_t debounce, IRType ...irProtocol>
inline uint8_t CIRLremote<debounce, irProtocol...>::
decodeSony12(const uint16_t duration) {
	// spaceTimeout gives some better accuracy, since we dont have a checksum here.
	// also set markTimeout if needed.
	const uint8_t irLength = SONY_LENGTH_12;
	const uint16_t timeoutThreshold = (SONY_TIMEOUT + SONY_MARK_LEAD) / 2;
	const uint16_t markLeadThreshold = (SONY_MARK_LEAD + SONY_MARK_ONE) / 2;
	const uint16_t markThreshold = (SONY_MARK_ONE + SONY_MARK_ZERO) / 2;
	const uint16_t markTimeout = 0; // (SONY_MARK_LEAD + SONY_MARK_ONE) / 2
	const uint16_t spaceTimeout = SONY_MARK_ONE;

	// if timeout always start next possible reading and abort any pending readings
	//TODO maybe use the sending 3 times to determine sony 12 vs 20
	// call the event from here after 2nd valid input if its still smaller than x
	if (duration >= timeoutThreshold)
		countSony12 = 1;

	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
	// this is to not conflict with other protocols while they are sending 0/1 which might be similar to a lead in another protocol
	else if (countSony12 == 0)
		return 0;

	// check pulses for mark/space and lead + logical 0/1 seperate
	else {
		// Mark pulses (odd numbers)
		if (countSony12 % 2 == 1) {
			// check Mark Lead (needs a timeout or a correct signal)
			if (countSony12 == 1) {
				// wrong lead
				if (duration < markLeadThreshold) {
					countSony12 = 0;
					return 0;
				}
			}

			else {
				// check for timeout if needed (might be a different protocol)
				if (markTimeout && duration > markTimeout) {
					countSony12 = 0;
					return 0;
				}

				// check different logical space pulses

				// get number of the Mark Bits (starting from zero)
				// only save every 2nd value, substract the first two lead pulses
				uint8_t length = (countSony12 / 2) - 1;

				// move bits and write 1 or 0 depending on the duration
				// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
				dataSony12[length / 8] >>= 1;
				// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
				if (duration >= markThreshold)
					dataSony12[length / 8] |= 0x80;

				// check last input (always a mark)
				if (countSony12 > irLength) {
					// reset reading
					countSony12 = 0;

					// Sony has no checksum
					return IR_SONY12;
				}
			}
		}

		// Space pulses (even numbers)
		else {
			// check for timeout if needed (might be a different protocol)
			// we could only check the lead or all data,
			// since the pulse should be always the same for lead and data
			// all data gives better errorcorrection and takes less flash
			if (spaceTimeout && duration > spaceTimeout) {
				countSony12 = 0;
				return 0;
			}
		}

		// next reading, no errors
		countSony12++;
	}
}


template <uint32_t debounce, IRType ...irProtocol>
inline void CIRLremote<debounce, irProtocol...>::
decodeSony20(const uint16_t duration) {
	//// pass the duration to the decoding function
	//bool newInput;
	//// 1st extra accuracy solution
	//if (sizeof...(irProtocol) != 1)
	//	newInput = IRLdecode <SONY_LENGTH_20, (SONY_TIMEOUT + SONY_MARK_LEAD) / 2, // irLength, timeoutThreshold
	//	(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, 0, // markLeadThreshold, spaceLeadThreshold
	//	0, (SONY_MARK_ONE + SONY_MARK_ZERO) / 2, // spaceLeadHoldingThreshold, markThreshold
	//	0, // spaceThreshold
	//	(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, SONY_MARK_ONE>// markTimeout, spaceTimeout
	//	(duration, dataSony20, countSony20);
	//else
	//	newInput = IRLdecode <SONY_LENGTH_20, (SONY_TIMEOUT + SONY_MARK_LEAD) / 2, // irLength, timeoutThreshold
	//	(SONY_MARK_LEAD + SONY_MARK_ONE) / 2, 0, // markLeadThreshold, spaceLeadThreshold
	//	0, (SONY_MARK_ONE + SONY_MARK_ZERO) / 2, // spaceLeadHoldingThreshold, markThreshold
	//	0, // spaceThreshold
	//	0, 0>// markTimeout, spaceTimeout
	//	(duration, dataSony20, countSony20);

	//if (newInput){
	//	// protocol has no checksum
	//	uint8_t upper5Bits = ((dataSony20[2] >> 2) & 0x3E);
	//	uint8_t lsb = (dataSony20[0] >> 7) & 0x01;
	//	uint16_t address = (upper5Bits << 8) | (dataSony20[1] << 1) | lsb;
	//	uint32_t command = dataSony20[0] & 0x7F;
	//	// 2nd extra accuracy solution
	//	//if ((sizeof...(irProtocol) != 1) && (address || command))
	//	IREvent(IR_SONY20, address, command);

	//	// reset reading
	//	countSony20 = 0;
	//}
}

//
//
//template <uint32_t debounce, IRType ...irProtocol>
//template <uint8_t irLength, uint16_t timeoutThreshold, uint16_t markLeadThreshold, uint16_t spaceLeadThreshold,
//	uint16_t spaceLeadHoldingThreshold, uint16_t markThreshold, uint16_t spaceThreshold,
//	uint16_t markTimeout, uint16_t spaceTimeout>
//	inline bool CIRLremote<debounce, irProtocol...>::
//	IRLdecode(uint16_t duration, uint8_t data[], uint8_t &count){
//
//	// if timeout always start next possible reading and abort any pending readings
//	if (duration >= timeoutThreshold)
//		count = 1;
//
//	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
//	// this is to not conflict with other protocols while they are sending 0/1 which might be similar to a lead in another protocol
//	else if (count == 0)
//		return false;
//
//	// check pulses for mark/space and lead + logical 0/1 seperate
//	else{
//		// Mark pulses (odd numbers)
//		if (count % 2 == 1){
//			// check Mark Lead (needs a timeout or a correct signal)
//			if (markLeadThreshold && count == 1){
//				// wrong lead
//				if (duration <= markLeadThreshold){
//					count = 0;
//					return false;
//				}
//			}
//
//			else{
//				// check for timeout if needed (might be a different protocol)
//				if (markTimeout && duration > markTimeout){
//					count = 0;
//					return false;
//				}
//
//				// only check values if the protocol has different logical space pulses
//				else if (markThreshold){
//
//					// get number of the Mark Bits (starting from zero)
//					uint8_t length;
//					// only save every 2nd value, substract the first two lead pulses
//					if (!spaceThreshold)
//						length = (count / 2) - 1;
//					// special case: spaces and marks both have data in the pulse
//					else length = count - 2;
//
//					// move bits and write 1 or 0 depending on the duration
//					// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
//					data[length / 8] >>= 1;
//					if (duration > markThreshold)
//						data[length / 8] |= 0x80;
//					//else // normally not needed through the bitshift
//					//	data[length / 8] &= ~0x80;
//				}
//
//				// check last input (always a mark)
//				if (count > irLength){
//					// reset by decoding function
//					//count = 0;
//					return true;
//				}
//			}
//		}
//
//		// Space pulses (even numbers)
//		else{
//			//check Space Lead/Space Holding
//			if (spaceLeadThreshold && count == 2){
//				// normal Space, next reading
//				if (duration > spaceLeadThreshold);
//
//				// Button holding (if supported by protocol)
//				else if (spaceLeadHoldingThreshold && duration > spaceLeadHoldingThreshold){
//					// call the holding function after
//					// count not resetted to read it afterwards
//					// next mark ignored due to detecting techniques
//					//count = 0;
//					return true;
//				}
//				// wrong space
//				else {
//					count = 0;
//					return false;
//				}
//			}
//			else{
//				// check for timeout if needed (might be a different protocol)
//				if (spaceTimeout && duration > spaceTimeout){
//					count = 0;
//					return false;
//				}
//
//				// only check values if the protocol has different logical space pulses
//				else if (spaceThreshold){
//
//					// get number of the Space Bits (starting from zero)
//					uint8_t length;
//					// only save every 2nd value, substract the first two lead pulses
//					if (!markThreshold)
//						length = (count / 2) - 2;
//					// special case: spaces and marks both have data in the pulse
//					else length = count - 2;
//
//					// move bits and write 1 or 0 depending on the duration
//					// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
//					data[length / 8] >>= 1;
//					if (duration > spaceThreshold)
//						data[length / 8] |= 0x80;
//					//else // normally not needed through the bitshift
//					//	data[length / 8] &= ~0x80;
//				}
//			}
//		}
//
//		// next reading, no errors
//		count++;
//	}
//
//	// no valid input (yet)
//	return false;
//}


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
CIRL_NEC CIRLremote<debounce, irProtocol...>::
Nec;

*/

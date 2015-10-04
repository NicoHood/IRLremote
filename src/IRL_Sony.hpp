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

// Include guard
#pragma once

//================================================================================
// Protocol Definitions
//================================================================================

//SONY 8, 12, 15, 20
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:8,^22200)
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:7,D:5,^45m)+
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:7,D:8,^45m)+
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:7,D:5,S:8,^45m) +
// Lead + Mark logic
#define SONY_HZ					40000
#define SONY_PULSE				600UL
#define SONY_BLOCKS_8			1
#define SONY_BLOCKS_12			2
#define SONY_BLOCKS_15			2
#define SONY_BLOCKS_20			3
#define SONY_ADDRESS_LENGTH_8	0
#define SONY_ADDRESS_LENGTH_12	5
#define SONY_ADDRESS_LENGTH_15	8
#define SONY_ADDRESS_LENGTH_20	13
#define SONY_COMMAND_LENGTH_8	8
#define SONY_COMMAND_LENGTH_12	7
#define SONY_COMMAND_LENGTH_15	7
#define SONY_COMMAND_LENGTH_20	7
#define SONY_LENGTH_8			(2 + (8-1) * 2) // 2 for lead + space, -1 for mark end, 8 bit
#define SONY_LENGTH_12			(2 + (7+5-1) * 2) // 2 for lead + space, -1 for mark end, 12 bit
#define SONY_LENGTH_15			(2 + (7+8-1) * 2) // 2 for lead + space, -1 for mark end, 15 bit
#define SONY_LENGTH_20			(2 + (7+5+8-1) * 2) // 2 for lead + space, -1 for mark end, 20 bit
#define SONY_TIMEOUT_8			22200
#define SONY_TIMEOUT			45000 // 12, 15, 20 have the same timeout
#define SONY_MARK_LEAD			(SONY_PULSE * 4)
#define SONY_SPACE_LEAD			(SONY_PULSE * 1)
#define SONY_SPACE_HOLDING		0 // no holding function in this protocol
#define SONY_MARK_ZERO			(SONY_PULSE * 1)
#define SONY_MARK_ONE			(SONY_PULSE * 2)
#define SONY_SPACE_ZERO			(SONY_PULSE * 1)
#define SONY_SPACE_ONE			(SONY_PULSE * 1)

//================================================================================
// Decoding Class
//================================================================================

class Sony : public CIRLData{
public:
	Sony(void){
		// Empty	
	}
	
	// Hide anything that is inside this class so the user dont accidently uses this class
	template<typename protocol, typename ...protocols>
	friend class CIRLremote;
	
private:
	static inline uint8_t getSingleFlag(void) __attribute__((always_inline));
	static inline void checkTimeout(void) __attribute__((always_inline));
	static inline bool available(void) __attribute__((always_inline));
	static inline void read(IR_data_t* data) __attribute__((always_inline));

	// Decode functions for a single protocol/multiprotocol for less/more accuration
	static inline void decodeSingle(const uint16_t &duration) __attribute__((always_inline));
	static inline void decode(const uint16_t &duration) __attribute__((always_inline));

protected:
	// Temporary buffer to hold bytes for decoding the protocols
	// not all of them are compiled, only the used ones
	static uint8_t countSony;
	static uint8_t dataSony[SONY_BLOCKS_12];
};


uint8_t Sony::getSingleFlag(void){
	return CHANGE; //TODO change to RISING and implement function
}


void Sony::checkTimeout(void){
	// Not used in this protocol
}


bool Sony::available(void)
{
	// Only return a value if this protocol has new data
	if(IRLProtocol == IR_SONY12) //TODO
		return true;
	else
		return false;	
}


void Sony::read(IR_data_t* data){
	// Only (over)write new data if this protocol received any data
	if(available()){
		//TODO
		// protocol has no checksum
		uint8_t upper4Bits = ((dataSony[1] >> 3) & 0x1E);
		if (dataSony[0] & 0x80)
			upper4Bits |= 0x01;

		data->address = upper4Bits;
		data->command = dataSony[0] & 0x7F;
		data->protocol = IRLProtocol;
		
		// Reset protocol for new reading
		IRLProtocol &= ~IR_NEW_PROTOCOL;
	}
}


void Sony::decodeSingle(const uint16_t &duration){
	// not implemented TODO
	decode(duration);
}


void Sony::decode(const uint16_t &duration) {
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
		countSony = 1;

	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
	// this is to not conflict with other protocols while they are sending 0/1 which might be similar to a lead in another protocol
	else if (countSony == 0)
		return;

	// check pulses for mark/space and lead + logical 0/1 seperate
	else {
		// Mark pulses (odd numbers)
		if (countSony % 2 == 1) {
			// check Mark Lead (needs a timeout or a correct signal)
			if (countSony == 1) {
				// wrong lead
				if (duration < markLeadThreshold) {
					countSony = 0;
					return;
				}
			}

			else {
				// check for timeout if needed (might be a different protocol)
				if (markTimeout && duration > markTimeout) {
					countSony = 0;
					return;
				}

				// check different logical space pulses

				// get number of the Mark Bits (starting from zero)
				// only save every 2nd value, substract the first two lead pulses
				uint8_t length = (countSony / 2) - 1;

				// move bits and write 1 or 0 depending on the duration
				// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
				dataSony[length / 8] >>= 1;
				// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
				if (duration >= markThreshold)
					dataSony[length / 8] |= 0x80;

				// check last input (always a mark)
				if (countSony > irLength) {
					// reset reading
					countSony = 0;

					// Sony has no checksum
					IRLProtocol = IR_SONY12;
					return;
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
				countSony = 0;
				return;
			}
		}

		// next reading, no errors
		countSony++;
	}
}


/*


template <IRType ...irProtocol>
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
//template <IRType ...irProtocol>
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

*/

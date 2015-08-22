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
// Protocol Definitions
//================================================================================

//NEC
//IRP notation: {38.4k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,-78,(16,-4,1,-173)*)
// Lead + Space logic
#define NEC_HZ				38000UL
#define NEC_PULSE			564UL
#define NEC_BLOCKS			4
#define NEC_ADDRESS_LENGTH	16
#define NEC_COMMAND_LENGTH	16
#define NEC_LENGTH			(2 + NEC_BLOCKS * 8 * 2) // 2 for lead + space, each block has 8bits: mark and space
#define NEC_TIMEOUT			(NEC_PULSE * 78UL)
#define NEC_TIMEOUT_HOLDING (NEC_PULSE * 173UL)
#define NEC_TIMEOUT_REPEAT	(NEC_TIMEOUT + NEC_MARK_LEAD + NEC_SPACE_LEAD \
							+ NEC_MARK_ZERO * 16UL + NEC_MARK_ONE * 16UL \
							+ NEC_SPACE_ZERO * 16UL + NEC_SPACE_ONE * 16UL)
#define NEC_MARK_LEAD		(NEC_PULSE * 16UL)
#define NEC_SPACE_LEAD		(NEC_PULSE * 8UL)
#define NEC_SPACE_HOLDING	(NEC_PULSE * 4UL)
#define NEC_MARK_ZERO		(NEC_PULSE * 1UL)
#define NEC_MARK_ONE		(NEC_PULSE * 1UL)
#define NEC_SPACE_ZERO		(NEC_PULSE * 1UL)
#define NEC_SPACE_ONE		(NEC_PULSE * 3UL)

//PANASONIC
//IRP notation: {37k,432}<1,-1|1,-3>(8,-4,3:8,1:8,D:8,S:8,F:8,(D^S^F):8,1,-173)+
// Lead + Space logic
#define PANASONIC_HZ				37000
#define PANASONIC_PULSE				432UL
#define PANASONIC_BLOCKS			6
#define PANASONIC_ADDRESS_LENGTH	16
#define PANASONIC_COMMAND_LENGTH	32
#define PANASONIC_LENGTH			(2 + PANASONIC_BLOCKS * 8 * 2) // 2 for lead + space, each block has 8bits: mark and space
#define PANASONIC_TIMEOUT			(PANASONIC_PULSE * 173)
#define PANASONIC_MARK_LEAD			(PANASONIC_PULSE * 8)
#define PANASONIC_SPACE_LEAD		(PANASONIC_PULSE * 4)
#define PANASONIC_SPACE_HOLDING		0 // no holding function in this protocol
#define PANASONIC_MARK_ZERO			(PANASONIC_PULSE * 1)
#define PANASONIC_MARK_ONE			(PANASONIC_PULSE * 1)
#define PANASONIC_SPACE_ZERO		(PANASONIC_PULSE * 1)
#define PANASONIC_SPACE_ONE			(PANASONIC_PULSE * 3)

/*
Panasonic pulse demonstration:

*---------|                        |------------|   |---------|   |---|   ... -|   |---------
*         |                        |            |   |         |   |   |   ...  |   |
*         |                        |            |   |         |   |   |   ...  |   |
*         |------------------------|            |---|         |---|   |-  ...  |---|
*         |       Lead Mark        | Lead Space |  Logical 1  | Log 0 |  Data  |End|

*/

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

class CIRL_NEC {
public:
	CIRL_NEC(void) {}

	//uint8_t dataNec[NEC_BLOCKS];
	uint8_t countNec;

	uint8_t getAddress(void) __attribute__((always_inline)) {
		//dataNec[3]++;
		return countNec;
		//return UINT16_AT_OFFSET(dataNec, 0);
	}

	// uint32_t getCommand(void) __attribute__((always_inline)) {
	// 	return UINT16_AT_OFFSET(dataNec, 2);
	// }

	// uint8_t decodeNecOnly(const uint16_t duration) __attribute__((always_inline)) {
	// 	// this decoding function only works on FALLING

	// 	// no special accuracy set at the moment, no conflict detected yet
	// 	// due to the checksum we got a good recognition
	// 	const uint8_t irLength = NEC_LENGTH / 2;
	// 	const uint16_t timeoutThreshold = (NEC_TIMEOUT + NEC_MARK_LEAD + NEC_SPACE_LEAD) / 2;
	// 	const uint16_t leadThreshold = (NEC_MARK_LEAD + NEC_SPACE_LEAD + NEC_MARK_LEAD + NEC_SPACE_HOLDING) / 2;
	// 	const uint16_t leadHoldingThreshold = (NEC_MARK_LEAD + NEC_SPACE_HOLDING + NEC_MARK_ONE + NEC_SPACE_ONE) / 2;
	// 	const uint16_t threshold = (NEC_MARK_ONE + NEC_SPACE_ONE + NEC_MARK_ZERO + NEC_SPACE_ZERO) / 2;

	// 	// if timeout always start next possible reading and abort any pending readings
	// 	if (duration >= timeoutThreshold)
	// 		countNec = 0;

	// 	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
	// 	// this is to not conflict with other protocols while they are sending 0/1
	// 	// which might be similar to a lead in this protocol
	// 	else if (countNec == 0)
	// 		return 0;

	// 	// check Mark Lead (needs a timeout or a correct signal)
	// 	else if (countNec == 1) {
	// 		// wrong lead
	// 		if (duration < leadHoldingThreshold) {
	// 			countNec = 0;
	// 			return 0;
	// 		}
	// 		else if (duration < leadThreshold) {
	// 			// reset reading
	// 			countNec = 0;

	// 			// received a Nec Repeat signal
	// 			// next mark (stop bit) ignored due to detecting techniques
	// 			return IR_NEC_REPEAT;
	// 		}
	// 		// else normal lead, next reading
	// 	}

	// 	// pulses (mark + space)
	// 	else {
	// 		// check different logical space pulses

	// 		// get number of the Bits (starting from zero)
	// 		// substract the first lead pulse
	// 		uint8_t length = countNec - 2;

	// 		// move bits and write 1 or 0 depending on the duration
	// 		// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
	// 		dataNec[length / 8] >>= 1;
	// 		// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
	// 		if (duration >= threshold)
	// 			dataNec[length / 8] |= 0x80;

	// 		// last bit (stop bit)
	// 		if (countNec >= irLength) {
	// 			// reset reading
	// 			countNec = 0;

	// 			// Check if the protcol's checksum is correct (byte 0 is the inverse of byte 1)
	// 			// normally NEC also check for the inverse of the address (byte 2 is the inverse of byte 3)
	// 			// newer remotes don't have this because of the wide used protocol all addresses were already used
	// 			// to make it less complicated it's left out and the user can check the command inverse himself if needed
	// 			if (uint8_t((dataNec[2] ^ (~dataNec[3]))) == 0x00)
	// 			{
	// 				// TODO if normal mode + extended wanted, also add in the 2nd decode function
	// 				if ((uint8_t((dataNec[0] ^ (~dataNec[1]))) == 0x00))
	// 					return IR_NEC;
	// 				return IR_NEC_EXTENDED;
	// 			}

	// 			// checksum incorrect
	// 			else
	// 				return 0;
	// 		}
	// 	}

	// 	// next reading, no errors
	// 	countNec++;
	// }

	// uint8_t decodeNec(const uint16_t duration) {
	// 	// no special accuracy set at the moment, no conflict detected yet
	// 	// due to the checksum we got a good recognition
	// 	const uint8_t irLength = NEC_LENGTH;
	// 	const uint16_t timeoutThreshold = (NEC_TIMEOUT + NEC_MARK_LEAD) / 2;
	// 	const uint16_t markLeadThreshold = (NEC_MARK_LEAD + NEC_SPACE_ONE) / 2;
	// 	const uint16_t spaceLeadThreshold = (NEC_SPACE_LEAD + NEC_SPACE_HOLDING) / 2;
	// 	const uint16_t spaceLeadHoldingThreshold = (NEC_SPACE_HOLDING + NEC_SPACE_ONE) / 2;
	// 	const uint16_t spaceThreshold = (NEC_SPACE_ONE + NEC_SPACE_ZERO) / 2;

	// 	// if timeout always start next possible reading and abort any pending readings
	// 	if (duration >= timeoutThreshold)
	// 		countNec = 0;

	// 	// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
	// 	// this is to not conflict with other protocols while they are sending 0/1
	// 	// which might be similar to a lead in this protocol
	// 	else if (countNec == 0)
	// 		return 0;

	// 	// check Mark Lead (needs a timeout or a correct signal)
	// 	else if (countNec == 1) {
	// 		// wrong lead
	// 		if (duration < markLeadThreshold) {
	// 			countNec = 0;
	// 			return 0;
	// 		}
	// 	}

	// 	//check Space Lead/Space Holding
	// 	else if (countNec == 2) {
	// 		// wrong space
	// 		if (duration < spaceLeadHoldingThreshold) {
	// 			countNec = 0;
	// 			return 0;
	// 		}

	// 		else if (duration < spaceLeadThreshold) {
	// 			// reset reading
	// 			countNec = 0;

	// 			// received a Nec Repeat signal
	// 			// next mark (stop bit) ignored due to detecting techniques
	// 			return IR_NEC_REPEAT;
	// 		}
	// 		// else normal Space, next reading
	// 	}

	// 	// last mark (stop bit)
	// 	else if (countNec > irLength) {
	// 		// reset reading
	// 		countNec = 0;

	// 		// Check if the protcol's checksum is correct (byte 0 is the inverse of byte 1)
	// 		// normally NEC also check for the inverse of the address (byte 2 is the inverse of byte 3)
	// 		// newer remotes don't have this because of the wide used protocol all addresses were already used
	// 		// to make it less complicated it's left out and the user can check the command inverse himself if needed
	// 		if (uint8_t((dataNec[2] ^ (~dataNec[3]))) == 0x00)
	// 		{
	// 			if ((uint8_t((dataNec[0] ^ (~dataNec[1]))) == 0x00))
	// 				return IR_NEC;
	// 			return IR_NEC_EXTENDED;
	// 		}

	// 		// checksum incorrect
	// 		else
	// 			return 0;
	// 	}

	// 	// Space pulses (even numbers)
	// 	else if (countNec % 2 == 0) {
	// 		// check different logical space pulses

	// 		// get number of the Space Bits (starting from zero)
	// 		// only save every 2nd value, substract the first two lead pulses
	// 		uint8_t length = (countNec / 2) - 2;

	// 		// move bits and write 1 or 0 depending on the duration
	// 		// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
	// 		dataNec[length / 8] >>= 1;
	// 		// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
	// 		if (duration >= spaceThreshold)
	// 			dataNec[length / 8] |= 0x80;
	// 	}
	// 	// Mark pulses (odd numbers)
	// 	// else ignored, always the same mark length

	// 	// next reading, no errors
	// 	countNec++;
	// }
};

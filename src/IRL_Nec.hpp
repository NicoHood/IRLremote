

// variadic template to choose the specific protocols that should be used
//template <uint32_t debounce>
class CIRLNec {
public:
	CIRLNec(void){
		// Empty	
	}

	bool begin(uint8_t pin) {
		// try to attach PinInterrupt first
		if (digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT)
			attachInterrupt(digitalPinToInterrupt(pin), decodeNecOnly, FALLING);

		// if PCINT library used, try to attach it
	#ifdef PCINT_VERSION
		else if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT)
			attachPCINT(digitalPinToPCINT(pin), decodeNecOnly, FALLING);
	#endif

		// return an error if none of them work
		else
			return false;

		// if it passes the attach function everything went okay.
		return true;
	}


	static bool available(void){
		// Only return a value if this protocol has new data
		if(IRLProtocol == IR_NEC || IRLProtocol == IR_NEC_EXTENDED || IRLProtocol == IR_NEC_REPEAT)
			return true;
		else
			return false;	
	}
	
	static void read(IR_data_t* data){
		// Only (over)write new data if this protocol received any data
		if(available()){
			// Reset protocol for new reading
			// Do this above so protocol numbers start at 1, not 129
			IRLProtocol &= ~IR_NEW_PROTOCOL;
		
			data->address = UINT16_AT_OFFSET(dataNec, 0);
			data->command = UINT16_AT_OFFSET(dataNec, 2);
			data->protocol = IRLProtocol;
		}
	}
	
	static IR_data_t read(void){
		// create the return data
		IR_data_t data = { 0 };
		read(&data);

		// return the new protocol information to the user
		return data;
	}
	
	static void reset(void){
		countNec = 0;
		// Only reset if we were the protocol which caused the event
		if(available())
			IRLProtocol &= ~IR_NEW_PROTOCOL;
	}

	static inline uint8_t getSingleFlag(void){
		return FALLING;
	}
	
	template <uint32_t debounce>
		static inline void test(void){
		
	}
	
		//TODO
	template <uint32_t debounce>
	static inline void decodeSingle2(uint16_t duration) __attribute__((always_inline));
	
	
	//TODO
	//template <uint32_t debounce>
	static inline void decodeSingle(uint16_t duration, const uint32_t debounce) __attribute__((always_inline)){

		// no special accuracy set at the moment, no conflict detected yet
		// due to the checksum we got a good recognition
		//TODO defines?
		const uint8_t irLength = NEC_LENGTH / 2;
		const uint16_t timeoutThreshold = (NEC_TIMEOUT + NEC_MARK_LEAD + NEC_SPACE_LEAD) / 2;
		const uint16_t leadThreshold = (NEC_MARK_LEAD + NEC_SPACE_LEAD + NEC_MARK_LEAD + NEC_SPACE_HOLDING) / 2;
		const uint16_t leadHoldingThreshold = (NEC_MARK_LEAD + NEC_SPACE_HOLDING + NEC_MARK_ONE + NEC_SPACE_ONE) / 2;
		const uint16_t threshold = (NEC_MARK_ONE + NEC_SPACE_ONE + NEC_MARK_ZERO + NEC_SPACE_ZERO) / 2;

		// if timeout always start next possible reading and abort any pending readings
		if (duration >= timeoutThreshold)
			countNec = 0;

		// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
		// this is to not conflict with other protocols while they are sending 0/1
		// which might be similar to a lead in this protocol
		else if (countNec == 0)
			return;

		// check Mark Lead (needs a timeout or a correct signal)
		else if (countNec == 1) {
			// wrong lead
			if (duration < leadHoldingThreshold) {
				countNec = 0;
				return;
			}
			else if (duration < leadThreshold) {
				// reset reading
				countNec = 0;

				// Check if last event timed out long enough
				// to not trigger wrong buttons (1 Nec signal timespawn)
				if ((IRLLastTime - IRLLastEvent) >= ((debounce * 1000UL) + NEC_TIMEOUT_REPEAT))
					return;
					
				uint8_t protocol = IRLProtocol | IR_NEW_PROTOCOL;

				// received a Nec Repeat signal
				// next mark (stop bit) ignored due to detecting techniques
				IRLProtocol = IR_NEC_REPEAT;
				return;
			}
			// else normal lead, next reading
		}

		// pulses (mark + space)
		else {
			// check different logical space pulses

			// get number of the Bits (starting from zero)
			// substract the first lead pulse
			uint8_t length = countNec - 2;

			// move bits and write 1 or 0 depending on the duration
			// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
			dataNec[length / 8] >>= 1;
			// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
			if (duration >= threshold)
				dataNec[length / 8] |= 0x80;

			// last bit (stop bit)
			if (countNec >= irLength) {
				// reset reading
				countNec = 0;

				// Check if the protcol's checksum is correct (byte 0 is the inverse of byte 1)
				// normally NEC also check for the inverse of the address (byte 2 is the inverse of byte 3)
				// newer remotes don't have this because of the wide used protocol all addresses were already used
				// to make it less complicated it's left out and the user can check the command inverse himself if needed
				if (uint8_t((dataNec[2] ^ (~dataNec[3]))) == 0x00)
				{
					// TODO if normal mode + extended wanted, also add in the 2nd decode function
					if ((uint8_t((dataNec[0] ^ (~dataNec[1]))) == 0x00)){
						IRLProtocol = IR_NEC;
						return;
					}
					else{
						IRLProtocol = IR_NEC_EXTENDED;
						return;
					}
				}

				// checksum incorrect
				else
					return;
			}
		}

		// next reading, no errors
		countNec++;
	}
	
	static void decodeNecOnly(void) {
		// This function is called by Pin(Change)Interrupt and only works on FALLING.

		// Block if the protocol is already recognized
		if (IRLProtocol)
			return;

		// Save the duration between the last reading
		uint32_t time = micros();
		uint32_t duration_32 = time - IRLLastTime;
		IRLLastTime = time;

		// calculate 16 bit duration. On overflow sets duration to a clear timeout
		uint16_t duration = 0xFFFF;
		if (duration_32 <= 0xFFFF)
			duration = duration_32;

		// no special accuracy set at the moment, no conflict detected yet
		// due to the checksum we got a good recognition
		//TODO defines?
		const uint8_t irLength = NEC_LENGTH / 2;
		const uint16_t timeoutThreshold = (NEC_TIMEOUT + NEC_MARK_LEAD + NEC_SPACE_LEAD) / 2;
		const uint16_t leadThreshold = (NEC_MARK_LEAD + NEC_SPACE_LEAD + NEC_MARK_LEAD + NEC_SPACE_HOLDING) / 2;
		const uint16_t leadHoldingThreshold = (NEC_MARK_LEAD + NEC_SPACE_HOLDING + NEC_MARK_ONE + NEC_SPACE_ONE) / 2;
		const uint16_t threshold = (NEC_MARK_ONE + NEC_SPACE_ONE + NEC_MARK_ZERO + NEC_SPACE_ZERO) / 2;

		// if timeout always start next possible reading and abort any pending readings
		if (duration >= timeoutThreshold)
			countNec = 0;

		// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
		// this is to not conflict with other protocols while they are sending 0/1
		// which might be similar to a lead in this protocol
		else if (countNec == 0)
			return;

		// check Mark Lead (needs a timeout or a correct signal)
		else if (countNec == 1) {
			// wrong lead
			if (duration < leadHoldingThreshold) {
				countNec = 0;
				return;
			}
			else if (duration < leadThreshold) {
				// reset reading
				countNec = 0;

				// received a Nec Repeat signal
				// next mark (stop bit) ignored due to detecting techniques
				IRLProtocol = IR_NEC_REPEAT;
				return;
			}
			// else normal lead, next reading
		}

		// pulses (mark + space)
		else {
			// check different logical space pulses

			// get number of the Bits (starting from zero)
			// substract the first lead pulse
			uint8_t length = countNec - 2;

			// move bits and write 1 or 0 depending on the duration
			// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
			dataNec[length / 8] >>= 1;
			// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
			if (duration >= threshold)
				dataNec[length / 8] |= 0x80;

			// last bit (stop bit)
			if (countNec >= irLength) {
				// reset reading
				countNec = 0;

				// Check if the protcol's checksum is correct (byte 0 is the inverse of byte 1)
				// normally NEC also check for the inverse of the address (byte 2 is the inverse of byte 3)
				// newer remotes don't have this because of the wide used protocol all addresses were already used
				// to make it less complicated it's left out and the user can check the command inverse himself if needed
				if (uint8_t((dataNec[2] ^ (~dataNec[3]))) == 0x00)
				{
					// TODO if normal mode + extended wanted, also add in the 2nd decode function
					if ((uint8_t((dataNec[0] ^ (~dataNec[1]))) == 0x00)){
						IRLProtocol = IR_NEC;
						return;
					}
					else{
						IRLProtocol = IR_NEC_EXTENDED;
						return;
					}
				}

				// checksum incorrect
				else
					return;
			}
		}

		// next reading, no errors
		countNec++;
	}

	static void decode(const uint16_t duration) {
		// no special accuracy set at the moment, no conflict detected yet
		// due to the checksum we got a good recognition
		const uint8_t irLength = NEC_LENGTH;
		const uint16_t timeoutThreshold = (NEC_TIMEOUT + NEC_MARK_LEAD) / 2;
		const uint16_t markLeadThreshold = (NEC_MARK_LEAD + NEC_SPACE_ONE) / 2;
		const uint16_t spaceLeadThreshold = (NEC_SPACE_LEAD + NEC_SPACE_HOLDING) / 2;
		const uint16_t spaceLeadHoldingThreshold = (NEC_SPACE_HOLDING + NEC_SPACE_ONE) / 2;
		const uint16_t spaceThreshold = (NEC_SPACE_ONE + NEC_SPACE_ZERO) / 2;

		// if timeout always start next possible reading and abort any pending readings
		if (duration >= timeoutThreshold)
			countNec = 0;

		// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
		// this is to not conflict with other protocols while they are sending 0/1
		// which might be similar to a lead in this protocol
		else if (countNec == 0)
			return;

		// check Mark Lead (needs a timeout or a correct signal)
		else if (countNec == 1) {
			// wrong lead
			if (duration < markLeadThreshold) {
				countNec = 0;
				return;
			}
		}

		//check Space Lead/Space Holding
		else if (countNec == 2) {
			// wrong space
			if (duration < spaceLeadHoldingThreshold) {
				countNec = 0;
				return;
			}

			else if (duration < spaceLeadThreshold) {
				// reset reading
				countNec = 0;

				// received a Nec Repeat signal
				// next mark (stop bit) ignored due to detecting techniques
				IRLProtocol = IR_NEC_REPEAT;
				return;
			}
			// else normal Space, next reading
		}

		// last mark (stop bit)
		else if (countNec > irLength) {
			// reset reading
			countNec = 0;

			// Check if the protcol's checksum is correct (byte 0 is the inverse of byte 1)
			// normally NEC also check for the inverse of the address (byte 2 is the inverse of byte 3)
			// newer remotes don't have this because of the wide used protocol all addresses were already used
			// to make it less complicated it's left out and the user can check the command inverse himself if needed
			if (uint8_t((dataNec[2] ^ (~dataNec[3]))) == 0x00)
			{
				if ((uint8_t((dataNec[0] ^ (~dataNec[1]))) == 0x00)){
					IRLProtocol = IR_NEC;
					return;
				}
				else{
					IRLProtocol = IR_NEC_EXTENDED;
					return;
				}
			}

			// checksum incorrect
			else
				return;
		}

		// Space pulses (even numbers)
		else if (countNec % 2 == 0) {
			// check different logical space pulses

			// get number of the Space Bits (starting from zero)
			// only save every 2nd value, substract the first two lead pulses
			uint8_t length = (countNec / 2) - 2;

			// move bits and write 1 or 0 depending on the duration
			// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
			dataNec[length / 8] >>= 1;
			// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
			if (duration >= spaceThreshold)
				dataNec[length / 8] |= 0x80;
		}
		// Mark pulses (odd numbers)
		// else ignored, always the same mark length

		// next reading, no errors
		countNec++;
	}

protected:
	// temporary buffer to hold bytes for decoding the protocols
	// not all of them are compiled, only the used ones
	static uint8_t dataNec[NEC_BLOCKS];
	static uint8_t countNec;
};

template <uint32_t debounce> void CIRLNec::decodeSingle2(uint16_t duration)
	{

		// no special accuracy set at the moment, no conflict detected yet
		// due to the checksum we got a good recognition
		//TODO defines?
		const uint8_t irLength = NEC_LENGTH / 2;
		const uint16_t timeoutThreshold = (NEC_TIMEOUT + NEC_MARK_LEAD + NEC_SPACE_LEAD) / 2;
		const uint16_t leadThreshold = (NEC_MARK_LEAD + NEC_SPACE_LEAD + NEC_MARK_LEAD + NEC_SPACE_HOLDING) / 2;
		const uint16_t leadHoldingThreshold = (NEC_MARK_LEAD + NEC_SPACE_HOLDING + NEC_MARK_ONE + NEC_SPACE_ONE) / 2;
		const uint16_t threshold = (NEC_MARK_ONE + NEC_SPACE_ONE + NEC_MARK_ZERO + NEC_SPACE_ZERO) / 2;

		// if timeout always start next possible reading and abort any pending readings
		if (duration >= timeoutThreshold)
			countNec = 0;

		// on a reset (error in decoding) we are waiting for a timeout to start a new reading again
		// this is to not conflict with other protocols while they are sending 0/1
		// which might be similar to a lead in this protocol
		else if (countNec == 0)
			return;

		// check Mark Lead (needs a timeout or a correct signal)
		else if (countNec == 1) {
			// wrong lead
			if (duration < leadHoldingThreshold) {
				countNec = 0;
				return;
			}
			else if (duration < leadThreshold) {
				// reset reading
				countNec = 0;

				// Check if last event timed out long enough
				// to not trigger wrong buttons (1 Nec signal timespawn)
				if ((IRLLastTime - IRLLastEvent) >= ((debounce * 1000UL) + NEC_TIMEOUT_REPEAT))
					return;
					
				uint8_t protocol = IRLProtocol | IR_NEW_PROTOCOL;

				// received a Nec Repeat signal
				// next mark (stop bit) ignored due to detecting techniques
				IRLProtocol = IR_NEC_REPEAT;
				return;
			}
			// else normal lead, next reading
		}

		// pulses (mark + space)
		else {
			// check different logical space pulses

			// get number of the Bits (starting from zero)
			// substract the first lead pulse
			uint8_t length = countNec - 2;

			// move bits and write 1 or 0 depending on the duration
			// 1.7: changed from MSB to LSB. somehow takes a bit more flash but is correct and easier to handle.
			dataNec[length / 8] >>= 1;
			// set bit if it's a logical 1. Setting zero not needed due to bitshifting.
			if (duration >= threshold)
				dataNec[length / 8] |= 0x80;

			// last bit (stop bit)
			if (countNec >= irLength) {
				// reset reading
				countNec = 0;

				// Check if the protcol's checksum is correct (byte 0 is the inverse of byte 1)
				// normally NEC also check for the inverse of the address (byte 2 is the inverse of byte 3)
				// newer remotes don't have this because of the wide used protocol all addresses were already used
				// to make it less complicated it's left out and the user can check the command inverse himself if needed
				if (uint8_t((dataNec[2] ^ (~dataNec[3]))) == 0x00)
				{
					// TODO if normal mode + extended wanted, also add in the 2nd decode function
					if ((uint8_t((dataNec[0] ^ (~dataNec[1]))) == 0x00)){
						IRLProtocol = IR_NEC;
						return;
					}
					else{
						IRLProtocol = IR_NEC_EXTENDED;
						return;
					}
				}

				// checksum incorrect
				else
					return;
			}
		}

		// next reading, no errors
		countNec++;
	}

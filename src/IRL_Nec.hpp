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

//================================================================================
// Decoding Class
//================================================================================

class Nec : public CIRLData{
public:
	Nec(void){
		// Empty
	}

	// Hide anything that is inside this class so the user dont accidently uses this class
	template<typename protocol, typename ...protocols>
	friend class CIRLremote;

private:
	static inline uint8_t getSingleFlag(void) __attribute__((always_inline));
	static inline bool requiresCheckTimeout(void) __attribute__((always_inline));
	static inline void checkTimeout(void) __attribute__((always_inline));
	static inline bool available(void) __attribute__((always_inline));
	static inline void read(IR_data_t* data) __attribute__((always_inline));
	static inline bool requiresReset(void) __attribute__((always_inline));
	static inline void reset(void) __attribute__((always_inline));

	// Decode functions for a single protocol/multiprotocol for less/more accuration
	static inline void decodeSingle(const uint16_t &duration) __attribute__((always_inline));
	static inline void decode(const uint16_t &duration) __attribute__((always_inline));

protected:
	// Temporary buffer to hold bytes for decoding the protocols
	// not all of them are compiled, only the used ones
	static uint8_t countNec;
	static uint8_t dataNec[NEC_BLOCKS];
};


uint8_t Nec::getSingleFlag(void){
	return FALLING;
}

bool Nec::requiresCheckTimeout(void){
	// Not used in this protocol
	return false;
}

void Nec::checkTimeout(void){
	// Not used in this protocol
}


bool Nec::available(void)
{
	// Only return a value if this protocol has new data
	if(IRLProtocol == IR_NEC || IRLProtocol == IR_NEC_EXTENDED || IRLProtocol == IR_NEC_REPEAT)
		return true;
	else
		return false;
}


void Nec::read(IR_data_t* data){
	// Only (over)write new data if this protocol received any data
	if(available()){
		data->address = ((uint16_t)dataNec[1] << 8) | ((uint16_t)dataNec[0]);
		data->command = ((uint16_t)dataNec[3] << 8) | ((uint16_t)dataNec[2]);
	}
}


bool Nec::requiresReset(void){
	// Not used in this protocol
	return false;
}


void Nec::reset(void){
	countNec = 0;
}


void Nec::decodeSingle(const uint16_t &duration){

	// No special accuracy set at the moment, no conflict detected yet
	// due to the checksum we got a good recognition
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
			if ((IRLLastTime - IRLLastEvent) >= NEC_TIMEOUT_REPEAT)
				return;

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
				// Differenciate between normal and extended NEC
				if ((uint8_t((dataNec[0] ^ (~dataNec[1]))) == 0x00))
					IRLProtocol = IR_NEC;
				else
					IRLProtocol = IR_NEC_EXTENDED;
			}

			return;
		}
	}

	// next reading, no errors
	countNec++;
}


void Nec::decode(const uint16_t &duration) {
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

			// Check if last event timed out long enough
			// to not trigger wrong buttons (1 Nec signal timespawn)
			if ((IRLLastTime - IRLLastEvent) >= NEC_TIMEOUT_REPEAT)
				return;

			// Check if last protocol was also NEC
			// (only for multiple protocol decoding)
			uint8_t lastProtocol = IRLProtocol | IR_NEW_PROTOCOL;
			if(lastProtocol == IR_NEC || lastProtocol == IR_NEC_EXTENDED || lastProtocol == IR_NEC_REPEAT){
				// Received a Nec Repeat signal
				// Next mark (stop bit) ignored due to decoding techniques
				IRLProtocol = IR_NEC_REPEAT;
			}
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
			// Differenciate between normal and extended NEC
			if ((uint8_t((dataNec[0] ^ (~dataNec[1]))) == 0x00))
				IRLProtocol = IR_NEC;
			else
				IRLProtocol = IR_NEC_EXTENDED;
		}

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


//================================================================================
// API Class
//================================================================================

typedef void(*NecEventCallback)(void);

template<const NecEventCallback callback, const uint16_t address = 0>
class CNecAPI : public CIRLData{
  private:
    // Differenciate between timeout types
    enum TimeoutType : uint8_t
    {
      NO_TIMEOUT, 	// Keydown
      TIMEOUT, 		// Key release with timeout
      NEXT_BUTTON, 	// Key release, pressed again
      NEW_BUTTON, 	// Key release, another key is pressed
    } NecTimeoutType;

    // Keep track which key was pressed/held down how often
    uint8_t lastCommand = 0;
    uint8_t lastPressCount = 0;
    uint8_t lastholdCount = 0;

  public:
	// Triggers when the button is released.
	// Anything else than a normal press indicates the key was released.
	// This occurs on a timeout, new button, same button press.
	// In most cases pressTimeout makes more sense.
	bool releaseButton (void) {
	  if (NecTimeoutType != NO_TIMEOUT) {
	    return true;
	  }
	  return false;
	}


	// Check if a key was released (via timeout or another key got pressed).
	// Return how often the key was pressed.
    uint8_t pressTimeout(void)
	{
      if (NecTimeoutType == TIMEOUT || NecTimeoutType == NEW_BUTTON) {
        return lastPressCount;
      }
      return 0;
    }


    uint8_t command(void) {
      return lastCommand;
    }


	// Returns the current button press streak.
	// How many times have you pressed again the same button?
    uint8_t pressCount(void) {
      return lastPressCount;
    }


	// Counts how long a button press streak has been holding down.
	// A debounce input param can be used to only count every Xth time.
    uint8_t holdDebounce(const uint8_t debounce = 4)
	{
      // Only recognize the actual keydown event
      if (NecTimeoutType == NO_TIMEOUT)
      {
        // No debounce desired (avoid zero division)
        if (!debounce) {
          return 1 + lastholdCount;
        }

        // Only recognize every xth event
        if (!(lastholdCount % debounce)) {
          return (lastholdCount / debounce) + 1;
        }
      }
      return 0;
    }


	// Reset the button press and hold count.
	// Attention: No release/timeout event will trigger!
	// This is important if you want to end a chain,
	// which starts again with the next press.
	// Differenciate between 1 or 2 presses is a good usecase.
	// Hold a button -> event A triggers
	// Hold the button again -> event B triggers
	// If A (with a longer hold count) was already triggered, reset the API
    void reset(void)
	{
      lastPressCount = 0;
      lastholdCount = 0;
    }


    void read(const IR_data_t data) {
      // Check if the correct protocol and address (optional) is used
      if (((data.protocol != IR_NEC) && (data.protocol != IR_NEC_EXTENDED) && (data.protocol != IR_NEC_REPEAT)) || (address && (data.address != address)))
      {
        // Call the remote function again once the keypress timed out (or another protocol/address was used)
        const uint32_t timeoutLimitUs = 500000UL;
        if (lastPressCount && ((data.protocol != IR_NO_PROTOCOL) || (timeout() > timeoutLimitUs)))
        {
          // Flag timeout event, that the key was released and the current chain is over
          NecTimeoutType = TIMEOUT;
          callback();

          // Reset the button press and hold count after a timeout
          lastPressCount = 0;
          lastholdCount = 0;
        }
        return;
      }

      // Only get the low byte of the command. This saves us a lot of flash.
      // The high byte just contains the checksum (which is already checked by the IRLRemote).
      auto newCommand = data.Nec.command8;

      // Count the first button press
      if ((data.protocol == IR_NEC) || (data.protocol == IR_NEC_EXTENDED))
      {
        // Only the same button press inside a short timespawn is recognized as keypress
        if (newCommand == lastCommand)
        {
          // Flag that the last button hold is over, the same key is held down again
          if (lastPressCount) {
            NecTimeoutType = NEXT_BUTTON;
            callback();
          }
          // Increase pressing streak
          if (lastPressCount < 255) {
            lastPressCount++;
          }
        }
        // Different button than before
        else
        {
          // Flag that the last button hold is over, a differnt key is now held down
          if (lastPressCount) {
            NecTimeoutType = NEW_BUTTON;
            callback();
          }
          lastPressCount = 1;
        }

        // Start a new series of button holding
        lastholdCount = 0;
      }
      // Count the button holding
      else {
      	// Abort if no first press was recognized (after reset)
      	if(!lastPressCount){
      		return;
      	}

      	// Increment holding count
        if (lastholdCount < 255) {
          lastholdCount++;
        }
      }

      // Call the remote function and flag that the event was just received
      lastCommand = newCommand;
      NecTimeoutType = NO_TIMEOUT;
      callback();
    }
};

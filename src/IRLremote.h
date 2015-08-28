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

// Software version
#define IRL_VERSION 190

#include <Arduino.h>

// Include PinChangeInterrupt library if available
#ifdef PCINT_VERSION
#include "PinChangeInterrupt.h"
#endif

// Delay_basic is only for avrs. With ARM sending is currently not possible
#ifdef ARDUINO_ARCH_AVR
#include <util/delay_basic.h>
#endif


#include "IRLprotocols.h" //TODO remove
// Include pre recorded IR codes from IR remotes
#include "IRLkeycodes.h"

//================================================================================
// Definitions
//================================================================================

// Definition to convert an uint8_t array to an uint16_t/uint32_t at any position (thx timeage!)
#define UINT16_AT_OFFSET(p_to_8, offset)    ((uint16_t)*((const uint16_t *)((p_to_8)+(offset))))
#define UINT32_AT_OFFSET(p_to_8, offset)    ((uint32_t)*((const uint32_t *)((p_to_8)+(offset))))

// Enum as unique number for each protocol
typedef enum IRType {
	IR_NO_PROTOCOL = 0x00,
	// If MSB is not sells the last received protocol is still saved
	
	// MSB tells that the protocol was read
	IR_NEW_PROTOCOL = 0x80,
	IR_USER, // 1
	IR_ALL, // 2
	IR_NEC, // ...
	IR_NEC_EXTENDED,
	IR_NEC_REPEAT,
	IR_PANASONIC,
	IR_SONY8,
	IR_SONY12,
	IR_SONY15,
	IR_SONY20,
	// add new protocols here
};

// Struct that is returned by the read() function
typedef struct IR_data_t {
	// Variables to save received data
	uint8_t protocol;
	uint16_t address;
	uint32_t command;

	//TODO add nec struct/panasonic with id, checsum etc
};

// Class to hold the data for all other classes
// This is needed to avoid global variables that everyone can access
// Since the protocols also need to use this data but the IRLremote class
// is a template there is no way to get this data back without passing
// a pointer. This data class is only accessible by the friend classes themselve.
class CIRLData{
public:
	CIRLData(){
		// Empty
	}

protected:

	// Data that all protocols need for decoding
	static uint8_t IRLProtocol;
	
	// Time values for the last interrupt and the last valid protocol
	static uint32_t IRLLastTime;
	static uint32_t IRLLastEvent;
};

//================================================================================
// Receive
//================================================================================
	
template<uint32_t debounce, typename protocol, typename ...protocols>
class CIRLremote : public CIRLData{
public:
	CIRLremote(void);
	
	// Attach the interrupt so IR signals are detected
	inline bool begin(uint8_t pin);
	inline bool end(uint8_t pin);

	// User access to the library data
	static inline bool available(void);
	static inline IR_data_t read(void);

private:
	
	// Interrupt function that is attached
	static inline void interrupt(void);
		
	static inline void nop(...) {
		// Little hack to take as many arguments as possible
	}
};

// Implementation inline, moved to another .hpp file
#include "IRLremoteReceive.hpp"

// Include all protocol implementations
#include "IRL_Nec.hpp"

//================================================================================
// Transmit
//================================================================================

// functions to send the protocol
template <IRType irType>
inline void IRLwrite(const uint8_t pin, uint16_t address, uint32_t command) __attribute__((always_inline));

template <uint8_t addressLength, uint8_t commandLength,
          uint16_t Hz, bool addressFirst,
          uint16_t markLead, uint16_t spaceLead,
          uint16_t markZero, uint16_t markOne,
          uint16_t spaceZero, uint16_t spaceOne>
inline void IRLsend(volatile uint8_t * outPort, uint8_t bitmask, uint16_t address, uint32_t command) __attribute__((always_inline));

// functions to set the pin high or low (with bitbang pwm)
inline void IRLmark(const uint16_t Hz, volatile uint8_t * outPort, uint8_t bitMask, uint16_t time) __attribute__((always_inline));
inline void IRLspace(volatile uint8_t * outPort, uint8_t bitMask, uint16_t time) __attribute__((always_inline));

// implementation inline, moved to another .hpp file
#include "IRLremoteTransmit.hpp"

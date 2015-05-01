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

// software version
#define IRL_VERSION 180

#include <Arduino.h>

// include PinChangeInterrupt library if available
#ifdef PCINT_VERSION
#include "PinChangeInterrupt.h"
#endif

// delay_basic is only for avrs. With ARM sending is currently not possible
#ifdef ARDUINO_ARCH_AVR
#include <util/delay_basic.h>
#endif

//================================================================================
// Definitions
//================================================================================

#include "IRLprotocols.h"

// missing 1.0.6 definition workaround
#ifndef NOT_AN_INTERRUPT
#define NOT_AN_INTERRUPT -1
#endif

typedef enum IRType{
	IR_NO_PROTOCOL, // 0
	IR_USER, // 1
	IR_ALL, // 2
	IR_NEC, // ...
	IR_PANASONIC,
	IR_SONY8,
	IR_SONY12,
	IR_SONY15,
	IR_SONY20,
	// add new protocols here
};


//================================================================================
// Receive
//================================================================================

// variadic template to choose the specific protocols that should be used
template <uint32_t debounce, IRType ...irProtocol>
class CIRLremote{
public:
	CIRLremote(void);

	// attach the interrupt so IR signals are detected
	bool begin(uint8_t pin);
	bool end(uint8_t pin);

	// user access to the libraries data
	static bool available(void) __attribute__((always_inline));
	static uint8_t getProtocol(void) __attribute__((always_inline));
	static uint16_t getAddress(void) __attribute__((always_inline));
	static uint32_t getCommand(void) __attribute__((always_inline));
	static void reset(void) __attribute__((always_inline));

protected:
	// interrupt function that is attached
	static void IRLinterrupt(void);

	// event functions on a valid protocol
	static void IREvent(uint8_t p, uint16_t a, uint32_t c);
	template <IRType ir> static void buttonHolding(void);

	// decode functions
	template <IRType ir> static void decode(uint16_t duration) __attribute__((always_inline));
	static void decodeNec(const uint16_t duration) __attribute__((always_inline));
	static void decodePanasonic(const uint16_t duration) __attribute__((always_inline));
	static void decodeSony12(const uint16_t duration) __attribute__((always_inline));

	// multifunctional template for receiving
	template <uint8_t irLength, uint16_t timeoutThreshold, uint16_t markLeadThreshold, uint16_t spaceLeadThreshold,
		uint16_t spaceLeadHoldingThreshold, uint16_t markThreshold, uint16_t spaceThreshold,
		uint16_t markTimeout, uint16_t spaceTimeout>
		static bool IRLdecode(uint16_t duration, uint8_t data[], uint8_t &count) __attribute__((always_inline));
	
	// variables to save received data
	static uint8_t protocol;
	static uint16_t address;
	static uint32_t command;

	// time values for the last interrupt and the last valid protocol
	static uint32_t lastTime;
	static uint32_t lastEvent;

	//TODO
	static uint32_t k[30];

	static void nop(...) {
		// little hack to take as many arguments as possible
		// to execute several functions for the analogPins
	}
};

// implementation inline, moved to another .hpp file
#include "IRLremoteReceive.hpp"


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

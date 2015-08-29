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

template<uint32_t debounce, typename protocol, typename ...protocols>
CIRLremote<debounce, protocol, protocols...>::
CIRLremote(void) {
	// Empty
}


template<uint32_t debounce, typename protocol, typename ...protocols>
bool CIRLremote<debounce, protocol, protocols...>::
begin(uint8_t pin)
{
	// For single protocols use a different flag
	uint8_t flag = CHANGE;
	if(sizeof...(protocols) == 0){
		flag = protocol::getSingleFlag();
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


template<uint32_t debounce, typename protocol, typename ...protocols>
bool CIRLremote<debounce, protocol, protocols...>::
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


template<uint32_t debounce, typename protocol, typename ...protocols>
bool CIRLremote<debounce, protocol, protocols...>::
available(void)
{
	// This if construct saves flash
	if(IRLProtocol & IR_NEW_PROTOCOL)
		return true;
	else 
		return false;
}


template<uint32_t debounce, typename protocol, typename ...protocols>
IR_data_t CIRLremote<debounce, protocol, protocols...>::
read(void)
{
	// If nothing was received return an empty struct
	IR_data_t data = { 0 };
	
	// Only the received protocol will write data into the struct
	cli();
	uint8_t oldSREG = SREG;
	
	protocol::read(&data);
	nop((protocols::read(&data), 0)...);
	
	SREG = oldSREG;
	sei();
	
	// Return the new protocol information to the user
	return data;
}


template<uint32_t debounce, typename protocol, typename ...protocols>
uint32_t CIRLremote<debounce, protocol, protocols...>::
lastEvent(void)
{
	// Return last event time (in micros)
	cli();
	uint8_t oldSREG = SREG;
	
	uint32_t time = IRLLastEvent;
	
	SREG = oldSREG;
	sei();
	
	return time; 
}


template<uint32_t debounce, typename protocol, typename ...protocols>
uint32_t CIRLremote<debounce, protocol, protocols...>::
timeout(void)
{
	// Return time between last event time (in micros)
	cli();
	uint8_t oldSREG = SREG;
	
	uint32_t timeout = micros() - IRLLastEvent;
	
	SREG = oldSREG;
	sei();
	
	return timeout; 
}


//================================================================================
// Interrupt Function
//================================================================================

template<uint32_t debounce, typename protocol, typename ...protocols>
void CIRLremote<debounce, protocol, protocols...>::
interrupt(void) 
{ 
	// Block if the protocol is already recognized
	if (available())
		return;

	// Save the duration between the last reading
	uint32_t time = micros();
	uint32_t duration_32 = time - IRLLastTime;
	IRLLastTime = time;

	// Calculate 16 bit duration. On overflow sets duration to a clear timeout
	uint16_t duration = 0xFFFF;
	if (duration_32 <= 0xFFFF)
		duration = duration_32;
	
	// Call the decoding functions(s).
	if(sizeof...(protocols) == 0){
		// For a single protocol use a simpler decode function
		// to get maximum speed + recognition and minimum flash size
		protocol::decodeSingle(duration, debounce);
	}
	else{
		// Try to call all protocols decode functions
		protocol::decode(duration, debounce);
		nop((protocols::decode(duration, debounce), 0)...);
	}
	
	// Check if the last signal was received too fast.
	if (debounce && available()) 
	{
		// Do not save the new time, to not block forever if the user is holding a button.
		// This way you can still realize things like: hold a button to increase the volume
		if ((IRLLastTime - IRLLastEvent) < (debounce * 1000UL)){
			// Last input received too fast, ignore this one
			IRLProtocol &= ~IR_NEW_PROTOCOL;
			return;
		}

		// New valid signal, save new time
		IRLLastEvent = IRLLastTime;
	}
}

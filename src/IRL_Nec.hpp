/*
Copyright (c) 2014-2016 NicoHood
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

//==============================================================================
// Nec Decoding Implementation
//==============================================================================

bool CNec::begin(uint8_t pin)
{
    // Get pin ready for reading.
    pinMode(pin, INPUT_PULLUP);

    // Try to attach PinInterrupt first
    if (digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT){
        attachInterrupt(digitalPinToInterrupt(pin), interrupt, FALLING);
        return true;
    }

    // If PinChangeInterrupt library is used, try to attach it
#ifdef PCINT_VERSION
    else if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT){
        attachPCINT(digitalPinToPCINT(pin), interrupt, FALLING);
        return true;
    }
#endif

    // Return an error if none of them work (pin has no Pin(Change)Interrupt)
    return false;
}


bool CNec::end(uint8_t pin)
{
    // Disable pullup.
    pinMode(pin, INPUT);

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


bool CNec::available(){
    return countNec > (NEC_LENGTH / 2);
}


Nec_data_t CNec::read()
{
    // If nothing was received return an empty struct
    Nec_data_t data = Nec_data_t();

    // Only the received protocol will write data into the struct
    uint8_t oldSREG = SREG;
    cli();

    // Check and get data if we have new. Don't overwrite on repeat.
    if (available())
    {
        // Set last ISR to current time.
        // This is required to not trigger a timeout afterwards
        // and read corrupted data. This might happen
        // if the reading loop is too slow.
        mlastTime = micros();

        data.address = ((uint16_t)dataNec[1] << 8) | ((uint16_t)dataNec[0]);
        data.command = dataNec[2];

        // Reset reading
        countNec = 0;
    }

    // Enable interrupt again, after we saved a copy of the variables
    SREG = oldSREG;

    // Return the new protocol information to the user
    return data;
}


uint32_t CNec::timeout(void)
{
    // Return time between last event time (in micros)
    uint32_t time = micros();

    uint8_t oldSREG = SREG;
    cli();

    uint32_t timeout = mlastEvent;

    SREG = oldSREG;

    timeout = time - timeout;

    return timeout;
}


uint32_t CNec::lastEvent(void)
{
    // Return last event time (in micros)
    uint8_t oldSREG = SREG;
    cli();

    uint32_t time = mlastEvent;

    SREG = oldSREG;

    return time;
}


uint32_t CNec::nextEvent(void)
{
    // Return when the next event can be expected.
    // Zero means at any time.
    // Attention! This value is a little bit too high in general.
    // Also for the first press it is even higher than it should.
    uint32_t time = timeout();

    if(time >= NEC_TIMESPAN_HOLDING) {
        return 0;
    }

    return NEC_TIMESPAN_HOLDING - time;
}


void CNec::interrupt(void)
{
    // Block if the protocol is already recognized
    uint8_t count = countNec;
    if (count > (NEC_LENGTH / 2)) {
        return;
    }

    // Save the duration between the last reading
    uint32_t time = micros();
    uint32_t duration_32 = time - mlastTime;
    mlastTime = time;

    // Calculate 16 bit duration. On overflow sets duration to a clear timeout
    uint16_t duration = duration_32;
    if (duration_32 > 0xFFFF) {
        duration = 0xFFFF;
    }

    // On a timeout abort pending readings and start next possible reading
    if (duration >= ((NEC_TIMEOUT + NEC_LOGICAL_LEAD) / 2)) {
        countNec = 0;
    }

    // On a reset (error in decoding) wait for a timeout to start a new reading
    // This is to not conflict with other protocols while they are sending 0/1
    // which might be similar to a lead in this protocol
    else if (count == 0) {
        return;
    }

    // Check Mark Lead (requires a timeout)
    else if (count == 1)
    {
        // Wrong lead
        if (duration < ((NEC_LOGICAL_HOLDING + NEC_LOGICAL_ONE) / 2))
        {
            countNec = 0;
            return;
        }
        // Check for a "button holding" lead
        else if (duration < ((NEC_LOGICAL_LEAD + NEC_LOGICAL_HOLDING) / 2))
        {
            // Abort if last valid button press is too long ago
            if ((mlastTime - mlastEvent) >= NEC_TIMEOUT_REPEAT)
            {
                countNec = 0;
                return;
            }

            // Flag repeat signal via "invalid" address and empty command
            dataNec[0] = 0xFF;
            dataNec[1] = 0xFF;
            dataNec[2] = 0x00;

            // Received a Nec Repeat signal
            // Next mark (stop bit) ignored due to detecting techniques
            countNec = (NEC_LENGTH / 2);
            mlastEvent = mlastTime;
        }
    }

    // Check different logical space pulses (mark + space)
    else
    {
        // Get number of the Bits (starting from zero)
        // Substract the first lead pulse
        uint8_t length = count - 2;

        // Move bits (MSB is zero)
        dataNec[length / 8] >>= 1;

        // Set MSB if it's a logical one
        if (duration >= ((NEC_LOGICAL_ONE + NEC_LOGICAL_ZERO) / 2)) {
            dataNec[length / 8] |= 0x80;
        }

        // Last bit (stop bit following)
        if (count >= (NEC_LENGTH / 2))
        {
            // Check if the protcol's command checksum is correct
            if (uint8_t((dataNec[2] ^ (~dataNec[3]))) == 0x00) {
                mlastEvent = mlastTime;
            }
            else {
                countNec = 0;
                return;
            }
        }
    }

    // Next reading, no errors
    countNec++;
}


//==============================================================================
// API Class
//==============================================================================

// Reads data from the nec protocol (if available) and processes it.
template<const NecEventCallback callback, const uint16_t address>
void CNecAPI<callback, address>::read(void) {
  auto data = CNec::read();

  // Check if the correct protocol and address (optional) is used
  bool firstCommand = data.address != 0xFFFF;
  if ((data.address == 0) || (address && firstCommand && (data.address != address)))
  {
    // Call the remote function again once the keypress timed out
    if (lastPressCount && (timeout() > getTimeout()))
    {
      // Flag timeout event, key was released and the current chain is over
      NecTimeoutType = TIMEOUT;
      callback();

      // Reset the button press and hold count after a timeout
      lastPressCount = 0;
      lastHoldCount = 0;
    }
    return;
  }

  // Count the first button press
  if (firstCommand)
  {
    // The same button was pressed twice in a short timespawn (500ms)
    if (data.command == lastCommand)
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
    lastHoldCount = 0;

    // Save the new command. On a repeat (below) don't safe it.
    lastCommand = data.command;
  }
  // Count the button holding
  else
  {
    // Abort if no first press was recognized (after reset)
    if(!lastPressCount){
        return;
    }

    // Increment holding count
    if (lastHoldCount < 255) {
      lastHoldCount++;
    }
  }

  // Call the remote function and flag that the event was just received
  NecTimeoutType = NO_TIMEOUT;
  callback();
}


template<const NecEventCallback callback, const uint16_t address>
uint8_t CNecAPI<callback, address>::command(void)
{
  return lastCommand;
}


// Returns the current button press streak.
// How many times have you pressed again the same button?
template<const NecEventCallback callback, const uint16_t address>
uint8_t CNecAPI<callback, address>::pressCount(void)
{
  return lastPressCount;
}


// Counts how long a button press streak has been holding down.
// A debounce input param can be used to only count every Xth time.
template<const NecEventCallback callback, const uint16_t address>
uint8_t CNecAPI<callback, address>::holdCount(const uint8_t debounce )
{
  // Only recognize the actual keydown event
  if (NecTimeoutType == NO_TIMEOUT)
  {
    // No debounce desired (avoid zero division)
    if (!debounce) {
      return 1 + lastHoldCount;
    }

    // Only recognize every xth event
    if (!(lastHoldCount % debounce)) {
      return (lastHoldCount / debounce) + 1;
    }
  }
  return 0;
}


template<const NecEventCallback callback, const uint16_t address>
constexpr uint32_t CNecAPI<callback, address>::getTimeout(void) {
    return NEC_API_PRESS_TIMEOUT;
}


// Check if a key was released (via timeout or another key got pressed).
// Return how often the key was pressed.
template<const NecEventCallback callback, const uint16_t address>
uint8_t CNecAPI<callback, address>::pressTimeout(void)
{
  if (NecTimeoutType == TIMEOUT || NecTimeoutType == NEW_BUTTON) {
    return lastPressCount;
  }
  return 0;
}


// Return when the next timeout triggers.
// Zero means it already timed out.
template<const NecEventCallback callback, const uint16_t address>
uint32_t CNecAPI<callback, address>::nextTimeout(void)
{
    uint32_t time = timeout();

    if(time >= getTimeout()) {
        return 0;
    }

    return getTimeout() - time;
}



// Triggers when the button is released.
// Anything else than a normal press indicates the key was released.
// This occurs on a timeout, new button, same button press.
// In most cases pressTimeout makes more sense.
template<const NecEventCallback callback, const uint16_t address>
bool CNecAPI<callback, address>::releaseButton(void)
{
  if (NecTimeoutType != NO_TIMEOUT) {
    return true;
  }
  return false;
}


// Reset the button press and hold count.
// Attention: No release/timeout event will trigger!
// This is important if you want to end a chain,
// which starts again with the next press.
// Differenciate between 1 or 2 presses is a good usecase.
// Hold a button -> event A triggers
// Hold the button again -> event B triggers
// If A (with a longer hold count) was already triggered, reset the API
template<const NecEventCallback callback, const uint16_t address>
void CNecAPI<callback, address>::reset(void)
{
  lastPressCount = 0;
  lastHoldCount = 0;
}

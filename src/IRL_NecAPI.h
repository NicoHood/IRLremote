/*
Copyright (c) 2014-2017 NicoHood
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

#include "IRL_Nec.h"

//==============================================================================
// API Class
//==============================================================================

typedef void(*NecEventCallback)(void);
#define NEC_API_PRESS_TIMEOUT (500UL * 1000UL)

template<const NecEventCallback callback, const uint16_t address = 0x0000>
class CNecAPI : public CNec
{
public:
    // User API to access library data
    inline void read(void);
    inline uint8_t command(void);
    inline uint8_t pressCount(void);
    inline uint8_t holdCount(const uint8_t debounce = 0);
    inline constexpr uint32_t getTimeout(void);
    inline uint8_t pressTimeout(void);
    inline uint32_t nextTimeout(void);
    inline bool releaseButton (void);
    inline void reset(void);

protected:
    // Differenciate between timeout types
    enum TimeoutType : uint8_t
    {
        NO_TIMEOUT,     // Keydown
        TIMEOUT,         // Key release with timeout
        NEXT_BUTTON,     // Key release, pressed again
        NEW_BUTTON,     // Key release, another key is pressed
    } NecTimeoutType;

    // Keep track which key was pressed/held down how often
    uint8_t lastCommand = 0;
    uint8_t lastPressCount = 0;
    uint8_t lastHoldCount = 0;
};

//==============================================================================
// API Class Implementation
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
    auto time = timeout();
    auto timeout = getTimeout();

    if(time >= timeout) {
        return 0;
    }

    return timeout - time;
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

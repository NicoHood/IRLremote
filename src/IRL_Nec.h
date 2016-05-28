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

#include <Arduino.h>
#include "IRL_Receive.h"
#include "IRL_Time.h"
#include "IRL_Protocol.h"

//==============================================================================
// Protocol Definitions
//==============================================================================

// NEC
// IRP notation:
// {38.4k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,-78,(16,-4,1,-173)*)
// Lead + Space logic
#define NEC_HZ                38000UL
#define NEC_PULSE             564UL
#define NEC_ADDRESS_LENGTH    16
#define NEC_COMMAND_LENGTH    16
#define NEC_DATA_LENGTH       (NEC_ADDRESS_LENGTH + NEC_COMMAND_LENGTH)
#define NEC_BLOCKS            (NEC_DATA_LENGTH / 8)
// 2 for lead + space, each block has mark and space
#define NEC_LENGTH            (2 + NEC_DATA_LENGTH * 2)
#define NEC_TIMEOUT           (NEC_PULSE * 78UL)
#define NEC_TIMEOUT_HOLDING   (NEC_PULSE * 173UL)
#define NEC_TIMESPAN_HOLDING  (NEC_TIMEOUT_HOLDING + NEC_LOGICAL_HOLDING)
#define NEC_TIMEOUT_REPEAT    (NEC_TIMESPAN_HOLDING * 3 / 2)
#define NEC_MARK_LEAD         (NEC_PULSE * 16UL)
#define NEC_SPACE_LEAD        (NEC_PULSE * 8UL)
#define NEC_SPACE_HOLDING     (NEC_PULSE * 4UL)
#define NEC_LOGICAL_LEAD      (NEC_MARK_LEAD + NEC_SPACE_LEAD)
#define NEC_LOGICAL_HOLDING   (NEC_MARK_LEAD + NEC_SPACE_HOLDING)
#define NEC_MARK_ZERO         (NEC_PULSE * 1UL)
#define NEC_MARK_ONE          (NEC_PULSE * 1UL)
#define NEC_SPACE_ZERO        (NEC_PULSE * 1UL)
#define NEC_SPACE_ONE         (NEC_PULSE * 3UL)
#define NEC_LOGICAL_ZERO      (NEC_MARK_ZERO + NEC_SPACE_ZERO)
#define NEC_LOGICAL_ONE       (NEC_MARK_ONE + NEC_SPACE_ONE)

typedef uint16_t Nec_address_t;
typedef uint8_t Nec_command_t;

// Struct that is returned by the read() function
struct Nec_data_t
{
    Nec_address_t address;
    Nec_command_t command;
};

//==============================================================================
// Nec Decoding Class
//==============================================================================

class CNec : public CIRL_Receive<CNec>,
             public CIRL_Time<CNec>,
             public CIRL_Protocol<CNec, Nec_data_t>
{
public:
    // User API to access library data
    inline bool available(void);
    static constexpr uint32_t timespanEvent = NEC_TIMESPAN_HOLDING;
    static constexpr uint8_t interruptMode = FALLING;

protected:
    friend CIRL_Receive<CNec>;
    friend CIRL_Protocol<CNec, Nec_data_t>;

    // Temporary buffer to hold bytes for decoding the protocol
    static volatile uint8_t countNec;
    static uint8_t dataNec[NEC_BLOCKS];

    // Protocol interface functions
    inline Nec_data_t getData(void);
    inline void resetReading(void);

    // Interrupt function that is attached
    static inline void interrupt(void);
};

extern CNec Nec;

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

// Include protocol implementation
#include "IRL_Nec.hpp"

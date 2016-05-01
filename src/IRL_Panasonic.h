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

// Delay_basic is only for avrs. With ARM sending is currently not possible
// TODO implement sending
#ifdef ARDUINO_ARCH_AVR
#include <util/delay_basic.h>
#endif

//==============================================================================
// Protocol Definitions
//==============================================================================

// PANASONIC
// IRP notation:
// {37k,432}<1,-1|1,-3>(8,-4,3:8,1:8,D:8,S:8,F:8,(D^S^F):8,1,-173)+
// Lead + Space logic
#define PANASONIC_HZ                37000
#define PANASONIC_PULSE             432UL
#define PANASONIC_ADDRESS_LENGTH    16
#define PANASONIC_COMMAND_LENGTH    32
#define PANASONIC_DATA_LENGTH       (PANASONIC_ADDRESS_LENGTH + \
                                    PANASONIC_COMMAND_LENGTH)
#define PANASONIC_BLOCKS            (PANASONIC_DATA_LENGTH / 8)
// 2 for lead + space, each block has mark and space
#define PANASONIC_LENGTH            (2 + PANASONIC_DATA_LENGTH * 2)
#define PANASONIC_TIMEOUT           (PANASONIC_PULSE * 173UL)
#define PANASONIC_MARK_LEAD         (PANASONIC_PULSE * 8UL)
#define PANASONIC_SPACE_LEAD        (PANASONIC_PULSE * 4UL)
#define PANASONIC_LOGICAL_LEAD      (PANASONIC_MARK_LEAD + PANASONIC_SPACE_LEAD)
// No holding function in this protocol
#define PANASONIC_MARK_ZERO         (PANASONIC_PULSE * 1UL)
#define PANASONIC_MARK_ONE          (PANASONIC_PULSE * 1UL)
#define PANASONIC_SPACE_ZERO        (PANASONIC_PULSE * 1UL)
#define PANASONIC_SPACE_ONE         (PANASONIC_PULSE * 3UL)
#define PANASONIC_LOGICAL_ZERO      (PANASONIC_MARK_ZERO + PANASONIC_SPACE_ZERO)
#define PANASONIC_LOGICAL_ONE       (PANASONIC_MARK_ONE + PANASONIC_SPACE_ONE)

/*
Panasonic pulse demonstration:

*---|                        |------------|   |---------|   |---|   ... -|   |--
*   |                        |            |   |         |   |   |   ...  |   |
*   |                        |            |   |         |   |   |   ...  |   |
*   |------------------------|            |---|         |---|   |-  ...  |---|
*   |       Lead Mark        | Lead Space |  Logical 1  | Log 0 |  Data  |End|
*/


// Enum as unique number for each protocol
enum Panasonic_type_t : uint8_t {
    IRL_PANASONIC_NO_PROTOCOL = 0x00,
    IRL_PANASONIC,
};


// Struct that is returned by the read() function
struct Panasonic_data_t
{
    Panasonic_type_t protocol;
    union {
        uint16_t address;
        uint16_t manufacturer;
    };
    union {
        uint32_t command;
        struct {
            uint32_t parity : 4;
            uint32_t system : 4;
            uint32_t product : 8;
            uint32_t function : 8;
            uint32_t checksum : 8;
        } japan;
        struct {
            uint32_t parity : 4;
            uint32_t genre1 : 4;
            uint32_t genre2 : 4;
            uint32_t data : 10;
            uint32_t id : 2;
            uint32_t checksum : 8;
        } denon;
    };
};


//==============================================================================
// Panasonic Decoding Class
//==============================================================================

class CPanasonic
{
public:
    // Attach the interrupt so IR signals are detected
    inline bool begin(uint8_t pin);
    inline bool end(uint8_t pin);

    // User API to access library data
    inline bool available(void);
    inline Panasonic_data_t read(void);
    inline uint32_t timeout(void);
    inline uint32_t lastEvent(void);

protected:
    // Enum as unique number for each protocol
    static volatile Panasonic_type_t protocol;

    // Temporary buffer to hold bytes for decoding the protocol
    static uint8_t countPanasonic;
    static uint8_t dataPanasonic[PANASONIC_BLOCKS];

    // Data that all protocols need for decoding
    static volatile uint8_t IRLProtocol;

    // Time values for the last interrupt and the last valid protocol
    static uint32_t mlastTime;
    static volatile uint32_t mlastEvent;

    // Interrupt function that is attached
    static inline void interrupt(void);
};

extern CPanasonic Panasonic;

//==============================================================================
// API Class
//==============================================================================

typedef void(*PanasonicEventCallback)(void);

template<const PanasonicEventCallback callback, const uint16_t address = 0x0000>
class CPanasonicAPI : public CPanasonic
{
public:
    // User API to access library data
    inline void read(void);
    inline uint8_t command(void);
    inline uint8_t pressCount(void);
    inline uint8_t holdCount(const uint8_t debounce = 0);
    inline uint8_t pressTimeout(void);
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
    } PanasonicTimeoutType;

    // Keep track which key was pressed/held down how often
    uint8_t lastCommand = 0;
    uint8_t lastPressCount = 0;
    uint8_t lastHoldCount = 0;
};

// Include protocol implementation
#include "IRL_Panasonic.hpp"

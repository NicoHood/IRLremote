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

#include "IRL_Receive.h"
#include "IRL_Time.h"
#include "IRL_Protocol.h"

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
#define PANASONIC_TIMESPAN_HOLDING  (PANASONIC_TIMEOUT + \
                                    PANASONIC_LOGICAL_LEAD + \
                                    (PANASONIC_DATA_LENGTH / 2) * \
                                    PANASONIC_LOGICAL_ONE + \
                                    (PANASONIC_DATA_LENGTH / 2) * \
                                    PANASONIC_LOGICAL_ZERO)
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

typedef uint16_t Panasonic_address_t;
typedef uint32_t Panasonic_command_t;

// Struct that is returned by the read() function
union Panasonic_data_t
{
    struct{
        union {
            Panasonic_address_t address;
            uint16_t manufacturer;
        };
        Panasonic_command_t command;
    };
    struct {
        union {
            uint16_t address;
            uint16_t manufacturer;
        };
        uint32_t parity : 4;
        uint32_t system : 4;
        uint32_t product : 8;
        uint32_t function : 8;
        uint32_t checksum : 8;
    } japan;
    struct {
        union {
            uint16_t address;
            uint16_t manufacturer;
        };
        uint32_t parity : 4;
        uint32_t genre1 : 4;
        uint32_t genre2 : 4;
        uint32_t data : 10;
        uint32_t id : 2;
        uint32_t checksum : 8;
    } denon;
};


//==============================================================================
// Panasonic Decoding Class
//==============================================================================

class CPanasonic : public CIRL_Receive<CPanasonic>,
                   public CIRL_Time<CPanasonic>,
                   public CIRL_Protocol<CPanasonic, Panasonic_data_t>
{
public:
    // User API to access library data
    inline bool available(void);
    inline constexpr uint32_t timespanEvent(void);

protected:
    friend CIRL_Receive<CPanasonic>;
    friend CIRL_Protocol<CPanasonic, Panasonic_data_t>;

    // Temporary buffer to hold bytes for decoding the protocol
    static volatile uint8_t countPanasonic;
    static uint8_t dataPanasonic[PANASONIC_BLOCKS];

    // Protocol interface functions
    inline Panasonic_data_t getData(void);
    inline void resetReading(void);

    // Interrupt function that is attached
    static inline void interrupt(void);
};

extern CPanasonic Panasonic;

// Include protocol implementation
#include "IRL_Panasonic.hpp"

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

//==============================================================================
// IRL_Time Class
//==============================================================================

template<class T>
class CIRL_Time
{
public:
    // User API to access library data
    inline uint32_t timeout(void);
    inline uint32_t lastEvent(void);
    inline uint32_t nextEvent(void);

    // Interface that is required to be implemented
    //static const uint32_t timespanEvent = VALUE;

protected:
    // Time mangement functions
    static inline uint16_t nextTime(void);

    // Time values for the last interrupt and the last valid protocol
    static uint32_t mlastTime;
    static volatile uint32_t mlastEvent;
};


//==============================================================================
// Static Data
//==============================================================================

// Protocol temporary data
template<class T> uint32_t CIRL_Time<T>::mlastTime = 0;
template<class T> volatile uint32_t CIRL_Time<T>::mlastEvent = 0;


//==============================================================================
// CIRL_Time Implementation
//==============================================================================

template<class T>
uint16_t CIRL_Time<T>::nextTime(void){
    // Save the duration between the last reading
    uint32_t time = micros();
    uint32_t duration_32 = time - mlastTime;
    mlastTime = time;

    // Calculate 16 bit duration. On overflow sets duration to a clear timeout
    uint16_t duration = duration_32;
    if (duration_32 > 0xFFFF) {
        duration = 0xFFFF;
    }

    return duration;
}


template<class T>
uint32_t CIRL_Time<T>::timeout(void)
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


template<class T>
uint32_t CIRL_Time<T>::lastEvent(void)
{
    // Return last event time (in micros)
    uint8_t oldSREG = SREG;
    cli();

    uint32_t time = mlastEvent;

    SREG = oldSREG;

    return time;
}


template<class T>
uint32_t CIRL_Time<T>::nextEvent(void)
{
    // Return when the next event can be expected.
    // Zero means at any time.
    // Attention! This value is a little bit too high in general.
    // Also for the first press it is even higher than it should.
    auto time = timeout();
    auto timespan = static_cast<T*>(this)->timespanEvent;

    if(time >= timespan) {
        return 0;
    }

    return timespan - time;
}

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

//==============================================================================
// Panasonic Decoding Implementation
//==============================================================================

bool CPanasonic::begin(uint8_t pin)
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


bool CPanasonic::end(uint8_t pin)
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


bool CPanasonic::available(){
    return protocol;
}


Panasonic_data_t CPanasonic::read()
{
    // If nothing was received return an empty struct
    Panasonic_data_t data = Panasonic_data_t();

    // Only the received protocol will write data into the struct
    uint8_t oldSREG = SREG;
    cli();

    // Check and get data if we have new. Don't overwrite on repeat.
    auto newprotocol = protocol;
    if(newprotocol == IRL_PANASONIC)
    {
        data.address = ((uint16_t)dataPanasonic[1] << 8) |
                       ((uint16_t)dataPanasonic[0]);
        data.command = ((uint32_t)dataPanasonic[5] << 24) |
                       ((uint32_t)dataPanasonic[4] << 16) |
                       ((uint32_t)dataPanasonic[3] << 8)  |
                       ((uint32_t)dataPanasonic[2]);
    }

    // Set last ISR to current time.
    // This is required to not trigger a timeout afterwards
    // and read corrupted data. This might happen
    // if the reading loop is too slow.
    if(newprotocol) {
        mlastTime = micros();

        // Save and remove new protocol flag
        data.protocol = protocol;
        protocol = IRL_PANASONIC_NO_PROTOCOL;
    }

    // Enable interrupt again, after we saved a copy of the variables
    SREG = oldSREG;

    // Return the new protocol information to the user
    return data;
}


uint32_t CPanasonic::timeout(void)
{
    // Return time between last event time (in micros)
    uint8_t oldSREG = SREG;
    cli();

    uint32_t timeout = micros() - mlastEvent;

    SREG = oldSREG;

    return timeout;
}


uint32_t CPanasonic::lastEvent(void)
{
    // Return last event time (in micros)
    uint8_t oldSREG = SREG;
    cli();

    uint32_t time = mlastEvent;

    SREG = oldSREG;

    return time;
}


void CPanasonic::interrupt(void)
{
    // Block if the protocol is already recognized
    if (protocol) {
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
    if (duration >= ((PANASONIC_TIMEOUT + PANASONIC_LOGICAL_LEAD) / 2)) {
        countPanasonic = 1;
    }

    // On a reset (error in decoding) wait for a timeout to start a new reading
    // This is to not conflict with other protocols while they are sending 0/1
    // which might be similar to a lead in this protocol
    else if (countPanasonic == 0) {
        return;
    }

    // Check Mark Lead (requires a timeout)
    else if (countPanasonic == 1)
    {
        // Wrong lead
        if (duration < ((PANASONIC_LOGICAL_LEAD + PANASONIC_LOGICAL_ONE) / 2))
        {
            countPanasonic = 0;
            return;
        }
        else {
            // Next reading, no errors
            countPanasonic++;
        }
    }

    // Check different logical space pulses (mark + space)
    else
    {
        // Get number of the Bits (starting from zero)
        // Substract the first lead pulse
        uint8_t length = countPanasonic - 2;

        // Move bits (MSB is zero)
        dataPanasonic[length / 8] >>= 1;

        // Set MSB if it's a logical one
        if (duration >= ((PANASONIC_LOGICAL_ONE + PANASONIC_LOGICAL_ZERO) / 2))
        {
            dataPanasonic[length / 8] |= 0x80;
        }

        // Last bit (stop bit)
        if (countPanasonic >= (PANASONIC_LENGTH / 2))
        {
            // Check if the protcol's checksum is correct
            uint8_t XOR1 = dataPanasonic[2] ^
                           dataPanasonic[3] ^
                           dataPanasonic[4];
            if (XOR1 == dataPanasonic[5])
            {
                // Check vendor nibble checksum (optional)
                //uint8_t XOR2 = dataPanasonic[0] ^ dataPanasonic[1];
                //if(((XOR2 & 0x0F) ^ (XOR2 >> 4)) == (dataPanasonic[2] & 0x0F))
                //{
                    protocol = IRL_PANASONIC;
                    mlastEvent = mlastTime;
                //}
            }

            // Reset reading
            countPanasonic = 0;
            return;
        }

        // Next reading, no errors
        countPanasonic++;
    }
}

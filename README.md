IRLremote
=========

![Infrared Picture](header.jpg)

New lightweight IR library with different, smarter implementation.

This library is way more efficient than the "standard" IR library from Ken Shirriff 
and should be a replacement of the library (sorry for that ;D).

**The main improvements are:**
* Faster decoding (on the fly)
* Huge Ram improvements (13 bytes ram to decode NEC)
* Huge Flash improvements (less than 1kb flash to decode NEC)
* Written in C but uses C++ templates
* Very accurate even when pointing in different directions
* Maximum error correction
* Uses pin interrupt function/No timer needed
* Receiving and sending possible
* IDE 1.5.7 compatible
* Easy to use/Customizable
* Perfect for Attinys

**Supported Protocols**
* NEC
* Panasonic 
* Ask me for more

**Planned features:**
* Sending function (for Panasonic)
* Remove/improve bit banging PWM (maybe with tone())
* Use PCInt (conflict with SoftSerial)

Installation/How to use
=======================

Download the zip, extract and remove the "-master" of the folder.
Install the library [as described here](http://arduino.cc/en/pmwiki.php?n=Guide/Libraries).

Try the examples to see how it works.
I recommend to check the example ReceiveInterrupt or ReceiveBlocking.

See this reference about choosing the right interrupt pin:
http://arduino.cc/en/pmwiki.php?n=Reference/AttachInterrupt

Choose your protocol from one of these options:
```cpp
typedef enum IRType{
	IR_NO_PROTOCOL, // 0
	IR_USER, // 1
	IR_ALL, // 2
	IR_NEC, // ...
	IR_PANASONIC,
	// add new protocols here
	IR_RAW,
};
```
Each protocol number (in the examples) is equal to one of these names, starting from zero.
So if you get Protocol 3 this means its NEC. By default the library tries to decode all known protocols.
You can save a lot of ram/flash/performance by using a fixed protocol like IR_NEC instead of IR_ALL in the begin function.

The IR_USER IRType is for custom protocols/protocol combinations. See advanced examples.

For sending see the SendSerial/Button examples. But its still under construction.

Informations about IR protocols can be found here (a bit hard to understand but try it if you want to create a new protocol).
You can also ask me to implement any new protocol, just file it as issue or contact me on my blog.
http://www.hifi-remote.com/johnsfine/DecodeIR.html#JVC-48

More projects + contact can be found here:
http://nicohood.wordpress.com/

How it works
============

The idea is: minimal implementation with maximal recognition.
You can decode more than one protocol at the same time.

The trick is to only check the border between logical zero and one
to terminate space/mark and rely on the lead/length/checksum as error correction.
Lets say a logical 0 is 500ms and 1 is 1000ms. Then the border would be 750ms
to get maximum recognition instead of using just 10%.

Other protocols use different timings, leads, length, checksums
so it shouldnt interfere with other protocols even with this method.

This gives the library very small implementation with maximum recognition.
You can point into almost every possible direction in the room without wrong signals.

It saves a lot of ram because it decodes the signals "on the fly" when an interrupt occurs.
Thatswhy you should not add too many protocols at once to exceed the time of the next signal.
However its so fast, its shouldnt make any difference since we are talking about ms, not us.

**In comparison to Ken's lib**, he records the signals (with timer interrupts) in a buffer which takes a lot of ram.
Then you need to check in the main loop if the buffer has any valid signals.
It checks every signal, thatswhy its slow and takes a lot of flash.
And it also checks about 10~20% from the original value. Lets say a pulse is 100ms. Then 80-120ms is valid.
Thatswhy the recognition is worse. And he also doesnt check the protocol intern error correction.
For example NEC has an inverse in the command: the first byte is the inverse of the 2nd byte. Its easy to filter wrong signals then.
So every protocol has its built in checksums, which we will use. And I also check for timeouts and start new readings if the signal timed out.
The only positive thing is that with the timer the pin is more flexible. However i will try to implement a PCINT version later.

For sending i decided to use Bitbang. This works on every MCU and on any PIN. He used proper timers,
but only PIN 3 is usable for sending (an interrupt pin). Bitbang might have problems with other interrupts but should work relyable.
You can turn off interrupts before sending if you like to ensure a proper sending.
Normal IR devices shouldnt complain about a bit intolerance in the pwm signal. Just try to keep interrupts short.

Check ReceiveNECLed for a minimal implementation example.
The code itself seems to be a bit organized but i had to implement a lot of functions inline
to get maximum optimization.

Version History
===============
```
1.5.1 Release (21.09.2014)
* improved Bitbang PWM
* fixed SendSerial example

1.5.0 Release (20.09.2014)
* huge Ram and Flash improvements
* new library structure
* compacter code/new structure
* more examples
* more than one protocol possible at the same time
* customizable decoding functions

1.4.7 Release (13.09.2014)
* changed NEC to template

1.4.6 Release (30.08.2014)
* fixed writing address + command bug
* added sending function for NEC
* added Led example

1.4.5 Release (30.08.2014)
* fixed raw protocol

1.4.4 Release (07.08.2014)
* added raw protocol (broken)

1.4.3 Release (06.08.2014)
* changed and improved a lot of stuff
* rearranged classes
* removed older versions

1.0 - 1.3 (17.03.2014 - 03.5.2014)
* Release and minor fixes
```

Licence and Copyright
=====================
If you use this library for any cool project let me know!

```
Copyright (c) 2014 NicoHood
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
```
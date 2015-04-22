IRLremote 1.7.4
===============

![Infrared Picture](header.jpg)

New lightweight IR library with different, smarter implementation.

This library is way more efficient than the "standard" IR library from Ken Shirriff.

**The main improvements are:**
* Faster decoding (on the fly)
* Huge Ram improvements (13 bytes ram to decode NEC)
* Huge Flash improvements (less than 1kb flash to decode NEC)
* Receiving and sending possible
* Very accurate even when pointing in different directions
* Easy to use/Customizable
* Maximum error correction
* Uses PinInterrupt or PinChangeInterrupts/No timer needed
* Usable on almost any pin
* Perfect for Attinys
* Written in C, only uses C++ templates
* IDE 1.6.x compatible

**Supported Protocols**
* NEC
* Panasonic 
* Sony 12
* Ask me for more

**Planned features:**
* Test sending functions (for Panasonic, Sony etc)
* Add more protocols
* Improve bit banging PWM?
* Add Raw dump + sending option + improve raw function
* Add High/Low compare for unknown protocols

[Comment for feedback on my blog post.](http://nicohood.wordpress.com/2014/09/20/new-lightweight-infrared-library-for-arduino/)

Installation/How to use
=======================

Download the zip, extract and remove the "-master" of the folder.
Install the library [as described here](http://arduino.cc/en/pmwiki.php?n=Guide/Libraries).

### Receiving

To use the receiving you have to choose a **[PinInterrupt](http://arduino.cc/en/pmwiki.php?n=Reference/AttachInterrupt)**
or **[PinChangeInterrupt](https://github.com/NicoHood/PinChangeInterrupt)** pin.
They work a bit different under the hood but the result for IR is the same.
In order to use PinChangeInterrupts you also have to [install the library](https://github.com/NicoHood/PinChangeInterrupt).

Try the examples to see how the API works.

One you know what protocol your remote uses, choose yours from one of these options and change it in the setup():
```cpp
typedef enum IRType{
	IR_NO_PROTOCOL, // 0
	IR_USER, // 1
	IR_ALL, // 2
	IR_NEC, // ...
	IR_PANASONIC,
	IR_SONY8, // soon
	IR_SONY12,
	IR_SONY15, // soon
	IR_SONY20, // soon
	// add new protocols here
	IR_RAW,
};
```
Each protocol number (Serial output in the examples) is equal to one of these names, starting from zero.
So if you get protocol 3 this means its NEC. By default the library tries to decode all known protocols.

**You can save a lot of ram/flash/performance by using a fixed protocol** like IR_NEC instead of IR_ALL in the begin function.
If you choose a single protocol, keep in mind that accuracy is then set very low to get **maximal recognition and less code size**.
If you want to use more protocols or keep away different IR input devices which might cause problems, see the **advanced custom receiving example**.

The IR_USER IRType is for custom protocols/protocol combinations. See advanced examples.
There is also an example for raw output and a PCINT version. **With the basic PCINT version you can also
decrease flash size even more and be more flexible with your sending pin!**

**Keep in mind that from v1.6 to v1.7 the bit order has changed
and command and address might be a different value now.**


### Sending

**For sending see the SendSerial/Button examples.** You first have to read the codes of your remote with one of the receiving examples.
Choose your protocol in the sending sketch and use your address and command if choice.

Sending for Panasonic and Sony12 is not confirmed to work, since I have no device here to test.
Let me know if it works!

**Keep in mind that from v1.6 to v1.7 the bit order has changed
and command and address might be a different value now.**

### Adding new protocols

Informations about IR protocols can be found [here](http://www.hifi-remote.com/johnsfine/DecodeIR.html)
(a bit hard to understand but try it if you want to create a new protocol).
=> Website is offline, see /dev for an offline version of the website

You can also ask me to implement any new protocol, just file it as issue on Github or contact me on my blog.

More projects + contact can be found here:
http://nicohood.wordpress.com/

How it works
============

**The idea is: minimal implementation with maximal recognition.
You can decode more than one protocol at the same time.**

The trick is to only check the border between logical zero and one
to terminate space/mark and rely on the lead/length/checksum as error correction.
Lets say a logical 0 is 500ms and 1 is 1000ms. Then the border would be 750ms
to get maximum recognition instead of using just 10%.

Other protocols use different timings, leads, length, checksums
so it shouldn't interfere with other protocols even with this method.

This gives the library very small implementation with maximum recognition.
You can point into almost every possible direction in the room without wrong signals.

It saves a lot of ram because it decodes the signals "on the fly" when an interrupt occurs.
That's why you should not add too many protocols at once to exceed the time of the next signal.
However its so fast, its shouldn't make any difference since we are talking about ms, not us.

**In comparison to Ken's lib**, he records the signals (with timer interrupts) in a buffer which takes a lot of ram.
Then you need to check in the main loop if the buffer has any valid signals.
It checks every signal, that's why its slow and takes a lot of flash.
And it also checks about 10~20% from the original value. Lets say a pulse is 100ms. Then 80-120ms is valid.
That's why the recognition is worse. And he also doesn't check the protocol intern error correction.
For example NEC has an inverse in the command: the first byte is the inverse of the 2nd byte. Its easy to filter wrong signals then.
So every protocol has its built in checksums, which we will use. And I also check for timeouts and start new readings if the signal timed out.
The only positive thing is that with the timer the pin is more flexible. However i will try to implement a PCINT version later.

For sending I decided to use Bitbang. This works on every MCU and on any PIN. He used proper timers,
but only PIN 3 is usable for sending (an interrupt pin). Bitbang might have problems with other interrupts but should work relyable.
You can turn off interrupts before sending if you like to ensure a proper sending.
Normal IR devices shouldn't complain about a bit intolerance in the PWM signal. Just try to keep interrupts short.

Check ReceiveNECLed for a minimal implementation example.
The code itself seems to be a bit organized but I had to implement a lot of functions inline
to get maximum optimization.

This text should not attack the library from Ken. It's a great library with a lot of work and the most used IR library yet.
It is just worth a comparison and might be still useful like the old SoftSerial against the new one.

Version History
===============
```
1.7.4 Release (19.04.2015)
* Updated examples
* Added PinChangeInterrupt example
* Removed NoBlocking API and integrated this into the examples
* Added IRL_VERSION definition
* Added library.properties
* Improved Raw Example

1.7.3 Release (27.11.2014)
* Fixed critical Typo in decoding function
* Fixed weak function variable type
* Updated Raw Example slightly

1.7.2 Release (18.11.2014)
* Added always inline macro
* Changed duration to 16 bit
* Added easier PCINT map definitions
* Fixed ARM compilation for receiving (sending is still not working)

1.7.1 Release (15.11.2014)
* Added 16u2 HoodLoader2 example

1.7 Release (15.11.2014)
* Changed IR bit order from MSB to correct LSB
 * This improved the overall handling and also reduced flash usage
* Improved, extended sending function
* Added Receive Send Example
* Added smaller Basic PCINT function

1.6 Release (14.11.2014)
* Reworked decoding template
* Added Sony 12 protocol
* Added PCINT example for advanced users

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

License and Copyright
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

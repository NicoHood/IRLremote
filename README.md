IRLremote
=========

New lightweight IR library with different implementation

The idea was: minimal implementation with maximal recognition.
The trick behind this is to only recognize one Protocol at the same time but with maximum error correction.
It is still possible to add more than one Protocol but if you add too many it will slow down and not work anymore.
On top of that the first possible hit is recognized as valid. You might get wrong values if the protocols are very similar.
This is because of maximum recognition and error correction. Its by design this way.
However there are only two implemented yet so this shouldnt be a problem.

This library is way more efficient than the "standard" IR library from Ken Shirriff.

**The main improvements are:**
* Faster decoding (on the fly)
* Huge Ram improvements
* Huge Flash improvements (less than 1kb)
* Very accurate even when pointing in different directions
* Maximum error correction
* Interrupt function
* No timer is needed
* Receiving and sending possible

**Supported Protocols**
* NEC
* Panasonic 
* Ask me for more

**Planned features:**
* Sending function (for Panasonic)
* remove bit banging PWM
* Use PCInt (conflict with SoftSerial)

Try the examples to see how it works. See this reference about choosing the right interrupt pin:
http://arduino.cc/en/pmwiki.php?n=Reference/AttachInterrupt

Informations about IR Protocols can be found here (a bit hard to understand but try it if you want to create a new Protocol)
http://www.hifi-remote.com/johnsfine/DecodeIR.html#JVC-48

More Projects + contact can be found here:
http://nicohood.wordpress.com/

Version History
===============
```
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
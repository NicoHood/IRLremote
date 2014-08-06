IRLremote
=========

New lightweight IR library with different implementation

The idea was: minimal implementation with maximal recognition.
The trick behind this is to only recognize one Protocol at the same time but with maximum error correction.

This library is way more efficient than the "standard" IR library from Ken Shirriff.

**The main improvements are:**
* Faster decoding (on the fly)
* Huge Ram improvements
* Huge Flash improvements (less than 1kb)
* Very accurate even when pointing in different directions
* Maximum error correction
* Interrupt function

**Supported Protocols**
* NEC
* Panasonic 
* Ask me for more

**Planned features:**
* Sending function

Try the examples to see how it works. See this reference about choosing the right interrupt pin:
http://arduino.cc/en/pmwiki.php?n=Reference/AttachInterrupt

Informations about IR Protocols can be found here (a bit hard to understand but try it if you want to create a new Protocol)
http://www.hifi-remote.com/johnsfine/DecodeIR.html#JVC-48

More Projects + contact can be found here:
http://nicohood.wordpress.com/

Version History
===============
```
1.4 Release (06.08.2014)
* changed and improved a lot of stuff
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
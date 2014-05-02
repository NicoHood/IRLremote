IRLremote2
==========

New lightweight IR library with different implementation

Only for receiving NEC or Panasonic signals. Try the example.

The idea was: minimal implementation with maximal recognition. The trick behind this is to only recognize one Protocol at the same time but with maximum errorcorrection. I did not implement a sending function or an address output. The code can be extended with other Protocols if you understood how it works. It also provides a very general Raw sniffer.

The Library takes less than 1kb sketch!!! (if you implement to your program it wont add more than 1kb. there is a basic sketch size at around 6kb for a leonardo. In my program i noticed an increasment under 1kb with a big siwtch around!)

Informations about IR Protocols can be found here (a bit hard to understand but try it if you want to create a new Protocol)
http://www.hifi-remote.com/johnsfine/DecodeIR.html#JVC-48

More Projects+contact can be found here:
http://nicohood.wordpress.com/

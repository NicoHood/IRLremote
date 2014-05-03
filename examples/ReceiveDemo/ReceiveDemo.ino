/*
  IRLremote3 Demo
 Copyright (c) 2014 NicoHood.  All right reserved.
 
 This is a simple Demo to show you how to read and print values received from the library.
 
 Some additional Information:
 *Have a look at the CIRLremote.h and define the values for your own protocol!
 Default Protocol is NEC, only Panasonic (JVC-48) is supported yet
 *Make sure your remote battery is full!

 */

#include "IRLremote3.h"
const int interruptIR = 4; 


void setup(){
  delay(1000);
  while (!Serial); // while the serial stream is not open, do nothing (for Leonardo) 
  Serial.begin(9600);
  Serial.println("Booting!");

  //IRLremote.enable(interruptIR);
  IRLremote.enable(interruptIR, irEvent);
  
  // raw, direct output
  // last values is always the timeout, followed by the last length
  // and the new input mark. Use a high Serial baud for this,
  // because it dosent use a buffer, just a very simple output
  //IRLremote.enableRAW(interruptIR);
}

void loop(){
  // this will always output the latest correct ir input.
  // no need to call a resume function but the command will be deleted again
  // NEC will output one byte command, Panasonic 2byte.
  // this will not output the address because i think its not needed.
  // if you want to send this stuff again you need to enable the debug decode
  // and note the address and checksums. I just use IR for recieving one remote
  // so i didnt implement a sending function or address support.
  // this should keep the size small.
  // button holding for nec will always output 0xFF
  //uint32_t ircode=IRLremote.read();
  //if(ircode) Serial.println(ircode, BIN);
}

// called when directly received correct IR Signal
// do not use print inside!
void irEvent(uint32_t ircode){
  Serial.println(ircode,HEX);
 }





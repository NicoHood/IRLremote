/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRReceiveRaw Demo
 Receives IR signals and prints raw values to the Serial.
 */

#include "IRLremote.h"

const int interruptIR = 0;
IRLprotocolRaw IRprotocol;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  // no function is attached, however this wont fire for raw mode nevertheless
  IRLremote.begin(interruptIR, IRprotocol);
}

void loop() {
  // check if raw buff is full or timed out
  if (IRprotocol.available()) {
    // print a mark
    Serial.println("==========");
    // go through the whole buffer and print values
    for (int i = 0; i < IRprotocol.available(); i++) {
      Serial.print(i);
      Serial.print(": ");
      Serial.print(IRprotocol.buffer[i], HEX);
      Serial.print(" ");
      Serial.println(IRprotocol.buffer[i]);
    }
    // reset for new reading
    IRprotocol.reset();
  }
}

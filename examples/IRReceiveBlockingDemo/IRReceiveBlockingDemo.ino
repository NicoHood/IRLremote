/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRReceiveBlockingDemo
 Receives IR signals and blocks until the data is read.
 */

#include "IRLremote.h"

// select your protocol here
IRLremoteNEC IRLremote;
//IRLremotePanasonic IRLremote;

const int interruptIR = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  // no function is attached
  IRLremote.begin(interruptIR);
}

void loop() {
  // this will get the first valid input and ignores any new until its read
  if (IRLremote.available()) {
    // print as much as you want in this function
    IR_Remote_Data_t IRData = IRLremote.read();
    Serial.print("Address:");
    Serial.println(IRData.address, HEX);
    Serial.print("Command:");
    Serial.println(IRData.command, HEX);
  }
}
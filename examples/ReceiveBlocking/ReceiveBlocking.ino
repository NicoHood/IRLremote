/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL ReceiveBlocking
 Receives IR signals and blocks until the data is read.
 */

#include "IRLremote.h"

// See readme to choose the right interrupt number
const int interruptIR = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  // choose your protocol here to reduce flash/ram/performance
  // see readme for more information
  IRLbegin<IR_ALL>(interruptIR);
}

void loop() {
  // This will get the first valid input, blocks and ignores any new until its read
  if (IRLavailable()) {
    // Print as much as you want in this function
    // See source to terminate what number is for each protocol
    Serial.print("Protocol:");
    Serial.println(IRLgetProtocol());
    Serial.print("Address:");
    Serial.println(IRLgetAddress(), HEX);
    Serial.print("Command:");
    Serial.println(IRLgetCommand(), HEX);

    // Resume reading to get new values
    IRLreset();
  }
}

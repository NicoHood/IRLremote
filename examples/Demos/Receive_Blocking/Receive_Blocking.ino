/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive_Blocking
 Receives IR signals and blocks until the data is read.
 */

#include "IRLremote.h"

// see readme to choose the right interrupt number
const int interruptIR = digitalPinToInterrupt(2);

void setup() {
  // start serial debug output
  Serial.begin(115200);
  Serial.println("Startup");

  // choose your protocol here to reduce flash/ram/performance
  // see readme for more information
  IRLbegin<IR_ALL>(interruptIR);
}

void loop() {
  // this will get the first valid input, and blocks until you reset below
  if (IRLavailable()) {
    // print as much as you want in this function
    // see source to terminate what number is for each protocol
    Serial.println();
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
/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL ReceiveAdvanced
 Receives IR signals with custom decoding function and blocks until the data is read.
 */

#include "IRLremote.h"

// See readme to choose the right interrupt number
const int interruptIR = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  // set protocol to user to use the decode function below
  IRLbegin<IR_USER>(interruptIR);
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

void decodeIR(const uint32_t duration) {
  // Called when directly received and interrupt CHANGE
  // Do not use Serial inside, it can crash your Arduino!

  // add the protocols you want to use here to reduce flash/ram/performance
  // you can use this to decode e.g. two remotes at the same time
  // but dont need to check all other protocols
  // calling this function takes some small overhead though
  decodeNec(duration);
  decodePanasonic(duration);
}

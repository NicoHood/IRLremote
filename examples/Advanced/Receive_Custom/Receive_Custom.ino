/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive_Custom
 Receives IR signals with custom decoding function and blocks until the data is read.
 */

#include "IRLremote.h"

// see readme to choose the right pin (with an interrupt!) for your Arduino board
const int pinReceiveIR = digitalPinToInterrupt(2);

void setup() {
  // start serial debug output
  Serial.begin(115200);
  Serial.println("Startup");

  // set protocol to user to use the decode function below
  IRLbegin<IR_USER>(pinReceiveIR);
}

void loop() {
  // this will get the first valid input, and blocks until you reset below
  if (IRLavailable()) {
    // print as much as you want in this function
    // see source to terminate what number is for each protocol
    Serial.print("Protocol:");
    Serial.println(IRLgetProtocol());
    Serial.print("Address:");
    Serial.println(IRLgetAddress(), HEX);
    Serial.print("Command:");
    Serial.println(IRLgetCommand(), HEX);

    // resume reading to get new values
    IRLreset();
  }
}

void decodeIR(const uint32_t duration) {
  // called when directly received an interrupt CHANGE.
  // do not use Serial inside, it can crash your program!

  // add the protocols you want to use here to reduce flash/ram/performance.
  // you can use this to decode e.g. two remotes at the same time
  // but dont need to check all of the other protocols.
  // calling this function takes some small overhead though.
  decodeNec<IR_EXTRA_ACCURACY>(duration);
  decodePanasonic<IR_EXTRA_ACCURACY>(duration);
}
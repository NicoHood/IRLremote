/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Transceive
 Receives IR signals and blocks until the data is read.
 On receiving a specific IR input it will send another IR signal out.
 */

#include "IRLremote.h"

// see readme to choose the right pin (with an interrupt!) for your Arduino board
const int pinReceiveIR = digitalPinToInterrupt(2);

// choose any pin to send IR signals
const int pinSendIR = 3;

void setup() {
  // start serial debug output
  Serial.begin(115200);
  Serial.println("Startup");

  // choose your protocol here to reduce flash/ram/performance
  // see readme for more information
  IRLbegin<IR_ALL>(pinReceiveIR);
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

    // check if the input was a specific signal and send another signal out
    if (IRLgetProtocol() == IR_PANASONIC && IRLgetAddress() == 0x2002 && IRLgetCommand() == 0x813D1CA0) {
      // send the data, no pin setting to OUTPUT needed
      Serial.println();
      Serial.println("Sending...");
      uint16_t address = 0x6361;
      uint32_t command = 0xFE01;

      IRLwrite<IR_NEC>(pinSendIR, address, command);
    }

    // resume reading to get new values
    IRLreset();
  }
}
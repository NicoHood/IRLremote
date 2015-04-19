/*
 Copyright (c) 2014-2015 NicoHood
 See the readme for credit to other people.

 IRL Receive PinInterrupt

 Receives IR signals and instantly calls an attached interrupt function.
 This may fire more than one time if you press a button too long, add a debounce.
 Dont use Serial or delay inside interrupts!

 The following pins are usable for PinInterrupt:
 Arduino Uno/Nano/Mini: 2, 3
 Arduino Mega: 2, 3, 18, 19, 20, 21
 Arduino Leonardo/Micro: 0, 1, 2, 3, 7
 HoodLoader2: No pin is usable
 Attiny 24/44/84: 8
 Attiny 25/45/85: 2
 ATmega644P/ATmega1284P: 10, 11
*/

#include "IRLremote.h"

// do you want to block until the last data is received
// or do you always want to update the data to the newest input
#define IRL_BLOCKING true

// choose a valid PinInterrupt pin of your Arduino board
#define pinIR 2

// temporary variables to save latest IR input
uint8_t IRProtocol = 0;
uint16_t IRAddress = 0;
uint32_t IRCommand = 0;

void setup() {
  // start serial debug output
  Serial.begin(115200);
  Serial.println("Startup");

  // choose your protocol here to reduce flash/ram/performance
  // see readme for more information
  attachInterrupt(digitalPinToInterrupt(pinIR), IRLinterrupt<IR_ALL>, CHANGE);
}

void loop() {
  // temporary disable interrupts and print newest input
  uint8_t oldSREG = SREG;
  cli();
  if (IRProtocol) {
    // print as much as you want in this function
    // see readme to terminate what number is for each protocol
    Serial.println();
    Serial.print("Protocol:");
    Serial.println(IRProtocol);
    Serial.print("Address:");
    Serial.println(IRAddress, HEX);
    Serial.print("Command:");
    Serial.println(IRCommand, HEX);

    // reset variable to not read the same value twice
    IRProtocol = 0;
  }
  SREG = oldSREG;
}

void IREvent(uint8_t protocol, uint16_t address, uint32_t command) {
  // called when directly received a valid IR signal.
  // do not use Serial inside, it can crash your program!

  // dont update value if blocking is enabled
  if (IRL_BLOCKING && !IRProtocol) {
    // update the values to the newest valid input
    IRProtocol = protocol;
    IRAddress = address;
    IRCommand = command;
  }
}

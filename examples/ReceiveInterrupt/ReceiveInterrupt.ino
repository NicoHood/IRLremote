/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL ReceiveInterrupt
 Receives IR signals and instantly calls an attached interrupt function.
 This may fire more than one time if you press a button too long, add a debounce.
 Dont use Serial inside the interrupt!
 */

#include "IRLremote.h"

// See readme to choose the right interrupt number
const int interruptIR = 0;

uint8_t IRProtocol = 0;
uint16_t IRAddress = 0;
uint32_t IRCommand = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  // choose your protocol here to reduce flash/ram/performance
  // see readme for more information
  IRLbegin<IR_ALL>(interruptIR);
}

void loop() {
  // Temporary disable interrupts and print newest input
  uint8_t oldSREG = SREG;
  cli();
  if (IRProtocol) {
    // Print as much as you want in this function
    // See source to terminate what number is for each protocol
    Serial.print("Protocol:");
    Serial.println(IRProtocol);
    Serial.print("Address:");
    Serial.println(IRAddress, HEX);
    Serial.print("Command:");
    Serial.println(IRCommand, HEX);
    // Reset variables to not read the same value twice
    IRProtocol = 0;
  }
  SREG = oldSREG;
}

void IREvent(uint8_t protocol, uint16_t address, uint32_t command) {
  // Called when directly received correct IR Signal
  // Do not use Serial inside, it can crash your Arduino!

  // Update the values to the newest valid input
  IRProtocol = protocol;
  IRAddress = address;
  IRCommand = command;
}
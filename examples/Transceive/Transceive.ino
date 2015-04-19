/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Transceive
 Receives IR signals and blocks until the data is read.
 On receiving a specific IR input it will send another IR signal out.
 See PinInterrupt or PinChangeInterrupt examples for basic usage.
 */

#include "IRLremote.h"

// do you want to block until the last data is received
// or do you always want to update the data to the newest input
#define IRL_BLOCKING true

// choose a valid PinChangeInterrupt pin of your Arduino board
#define pinIR 2
#define pinSendIR 3

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

    // check if the input was a specific signal and send another signal out
    if (IRProtocol == IR_PANASONIC && IRAddress == 0x2002 && IRCommand == 0x813D1CA0) {
      // send the data, no pin setting to OUTPUT needed
      Serial.println();
      Serial.println("Sending...");
      uint16_t address = 0x6361;
      uint32_t command = 0xFE01;

      IRLwrite<IR_NEC>(pinSendIR, address, command);
    }

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

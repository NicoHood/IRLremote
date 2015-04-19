/*
 Copyright (c) 2014-2015 NicoHood
 See the readme for credit to other people.

 IRL Receive PinChangeInterrupt

 In order to use this example those two libraries are required:
 https://github.com/NicoHood/IRLremote
 https://github.com/NicoHood/PinChangeInterrupt

 Receives IR signals and instantly calls an attached interrupt function.
 This may fire more than one time if you press a button too long, add a debounce.

 PinChangeInterrupts are different than normal Interrupts.
 See readme for more information.
 Dont use Serial or delay inside interrupts!
 This library is not compatible with SoftSerial.

 The following pins are usable for PinChangeInterrupt:
 Arduino Uno/Nano/Mini: All pins are usable
 Arduino Mega: 10, 11, 12, 13, 50, 51, 52, 53, A8 (62), A9 (63), A10 (64),
               A11 (65), A12 (66), A13 (67), A14 (68), A15 (69)
 Arduino Leonardo/Micro: 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI)
 HoodLoader2: All (broken out 1-7) pins are usable
 Attiny 24/44/84: All pins are usable
 Attiny 25/45/85: All pins are usable
 ATmega644P/ATmega1284P: All pins are usable
*/

#include "IRLremote.h"
#include "PinChangeInterrupt.h"

// do you want to block until the last data is received
// or do you always want to update the data to the newest input
#define IRL_BLOCKING true

// choose a valid PinChangeInterrupt pin of your Arduino board
#define pinIR 10

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
  attachPCINT(digitalPinToPCINT(pinIR), IRLinterrupt<IR_ALL>, CHANGE);
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

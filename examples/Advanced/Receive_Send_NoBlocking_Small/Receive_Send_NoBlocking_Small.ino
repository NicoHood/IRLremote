/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive_Send_NoBlocking_Small
 Receives IR signals and blocks until the data is read.
 On receiving a specific IR input it will send another IR signal out.

 This example has no Serial output, so it takes not much flash.
 The trigger variable is used to not block in the ISR too long.
 */

#include "IRLremote.h"

// see readme to choose the right pin (with an interrupt!) for your Arduino board
const int pinReceiveIR = digitalPinToInterrupt(2);

// choose any pin to send IR signals
const int pinSendIR = 3;

// variable to save a sending trigger
bool sendNow = false;

void setup() {
  // choose your protocol here to reduce flash/ram/performance
  // see readme for more information
  IRLbegin<IR_PANASONIC>(pinReceiveIR);
}

void loop() {
  // check if the input was a specific signal and send another signal out
  if (sendNow) {
    // send the data, no pin setting to OUTPUT needed
    uint16_t address = 0x6361;
    uint32_t command = 0xFE01;

    IRLwrite<IR_NEC>(pinSendIR, address, command);

    // reset trigger
    sendNow = false;
  }
}

void IREvent(uint8_t protocol, uint16_t address, uint32_t command) {
  // called when directly received an interrupt CHANGE.
  // do not use Serial inside, it can crash your program!

  // set the trigger if input signal is the choosen
  if (protocol == IR_PANASONIC && address == 0x2002 && command == 0x813D1CA0)
    sendNow = true;
}

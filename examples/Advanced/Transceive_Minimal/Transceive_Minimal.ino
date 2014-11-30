/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Transceive Minimal
 Receives IR signals and blocks until the data is read.
 On receiving a specific IR input it will send another IR signal out.

 This example is a minimal implementation of the library with a
 receiving and sending part. Please see the Demos on how to use
 the library API or the PCINT functions.
 The trigger variable is used to not block in the ISR too long.
 Avoid big function calls inside the ISR!

 Flash usage (Uno):
 1,310 bytes
 Ram usage (Uno):
 21 bytes
 */

#include "IRLremote.h"

//================================================================================
// PCINT functions
//================================================================================

// PCINT definitions, edit and see data sheets for more information
#define PCINT_PIN 11

#define PCMSK *digitalPinToPCMSK(PCINT_PIN)
#define PCINT digitalPinToPCMSKbit(PCINT_PIN)
#define PCIE  digitalPinToPCICRbit(PCINT_PIN)
#define PCPIN *portInputRegister(digitalPinToPort(PCINT_PIN))

#if (PCIE == 0)
#define PCINT_vect PCINT0_vect
#elif (PCIE == 1)
#define PCINT_vect PCINT1_vect
#elif (PCIE == 2)
#define PCINT_vect PCINT2_vect
#else
#error This board doesnt support PCINT ?
#endif

// choose Panasonic for this demonstration
#define PCINT_FUNCTION IRLinterrupt<IR_PANASONIC>

ISR(PCINT_vect) {
  // that is the smallest possible version for a single pin.
  // this wont work if you set more than one PCINT
  // see the basic PCINT example for a correct version.
  PCINT_FUNCTION();
}

//================================================================================
// Main sketch
//================================================================================

// choose any pin to send IR signals
const int pinSendIR = 3;

// variable to save a sending trigger
bool sendNow = false;

void setup() {
  // pin change mask registers decide which pins are enabled as triggers
  PCMSK |= (1 << PCINT);

  // PCICR: Pin Change Interrupt Control Register - enables interrupt vectors
  PCICR |= (1 << PCIE);
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
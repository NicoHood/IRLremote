/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive_PinChangeInterrupt_Basic
 Receives IR signals with custom Pin Change Interrupt function (blocking).

 This is the very basic PCINT version to use almost any pin for the IRLremote.
 PCINT is useful if you are running out of normal INTs or if you are using HoodLoader2.
 Keep in mind that this PCINT is not compatible with SoftSerial (at the moment).

 On Arduino Uno you can use any pin for receiving.
 On Arduino Mega you can use 10, 11, 12, 13, 14, 15, 50, 51, 52, 53, A8 (62), A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14 (68), A15 (69).
 On Arduino Leonardo you can use 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).
 With HoodLoader2 you can use pin 1-7 and some other pins which are normally not connected.

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

#define PCINT_FUNCTION IRLinterrupt<IR_ALL>

// variable to save the last register state
volatile static uint8_t lastRegister;

void attachPinChangeInterrupt(void) {
  // save the current state first
  lastRegister = PCPIN;

  // pin change mask registers decide which pins are enabled as triggers
  PCMSK |= (1 << PCINT);

  // PCICR: Pin Change Interrupt Control Register - enables interrupt vectors
  PCICR |= (1 << PCIE);
}

void detachPinChangeInterrupt(void) {
  // disable the mask.
  PCMSK &= ~(1 << PCINT);

  // if that's the last one, disable the interrupt.
  if (PCMSK == 0)
    PCICR &= ~(0x01 << PCIE);
}

ISR(PCINT_vect) {
  // get the new pin states for port, compare with the old state and save new state
  // this is not needed if you only use a single PCINT pin
  // to be complete i've added it anyways. The smaller version would be just to call PCINT_FUNCTION();
  uint8_t currRegister = PCPIN;
  uint8_t changedPinMask = currRegister ^ lastRegister;
  lastRegister = currRegister;

  // if our needed pin has changed, call the IRL interrupt function
  if (changedPinMask & (1 << PCINT))
    PCINT_FUNCTION();
}

//================================================================================
// Main sketch
//================================================================================

void setup()
{
  // start serial debug output
  Serial.begin(115200);
  Serial.println(F("Startup"));

  // attach the new PinChangeInterrupt function above
  attachPinChangeInterrupt();
}

void loop() {
  // this will get the first valid input, and blocks until you reset below
  if (IRLavailable()) {
    // print as much as you want in this function
    // see source to terminate what number is for each protocol
    Serial.println();
    Serial.print(F("Protocol:"));
    Serial.println(IRLgetProtocol());
    Serial.print(F("Address:"));
    Serial.println(IRLgetAddress(), HEX);
    Serial.print(F("Command:"));
    Serial.println(IRLgetCommand(), HEX);

    // resume reading to get new values
    IRLreset();
  }
}
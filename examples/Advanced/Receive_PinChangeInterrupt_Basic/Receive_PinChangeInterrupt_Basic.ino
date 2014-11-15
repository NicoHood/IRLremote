/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive_PinChangeInterrupt_Basic
 Receives IR signals with custom Pin Change Interrupt function.

 This is the very basic PCINT version to use almost any pin for the IRLremote.
 All you need to do is look up the ports in the datasheet.
 Keep in mind that this PCINT is not compatible with SoftSerial (at the moment).
 */

#include "IRLremote.h"

//================================================================================
// PCINT functions
//================================================================================

// PCINT definitions, edit and see datasheets for more information
// example: pin 8 on Arduino Uno
#define PCMSK PCMSK0
#define PCINT PCINT0
#define PCIE  PCIE0
#define PCPIN PINB
#define PCINT_vect PCINT0_vect
#define PCINT_FUNCTION IRLinterrupt<IR_ALL>()

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
  uint8_t currRegister = PCPIN;
  uint8_t changedPinMask = currRegister ^ lastRegister;
  lastRegister = currRegister;

  // if our needed pin has changed, call the IRL interrupt function
  if (changedPinMask & (1 << PCINT))
    PCINT_FUNCTION;
}

//================================================================================
// Main sketch
//================================================================================

void setup()
{
  // start serial debug output
  Serial.begin(115200);
  Serial.println("Startup");

  // attach the new PinChangeInterrupt function above
  attachPinChangeInterrupt();
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

    // resume reading to get new values
    IRLreset();
  }
}

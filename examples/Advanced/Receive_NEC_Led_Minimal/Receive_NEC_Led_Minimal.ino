/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive NEC Led Minimal
 Receives IR and lights the LED on a valid signal.

 This should only demonstrate that you can use the IR sensor
 from almost every direction with almost no flash/ram.
 This method uses a minimal implementation of the PCINT example.
 Please see the Demos on how to use the library API or the
 PCINT functions. This sketch is only a minimal implementation.

 Flash usage (Uno):
 BlinkWithoutDelay:   1,006 bytes flash, 15 bytes ram
 IRL Receive NEC Led: 1,390 bytes flash, 22 bytes ram

 Ram usage (Uno):
 4bytes lastTime
 for each protocol:
 4-6 bytes temporary work data
 1 byte temporary count
 = 4+(n*(5~7)) bytes ram used
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

// choose NEC for this demonstration (most common protocol)
#define PCINT_FUNCTION IRLinterrupt<IR_NEC>

ISR(PCINT_vect) {
  // that is the smallest possible version for a single pin.
  // this wont work if you set more than one PCINT
  // see the basic PCINT example for a correct version.
  PCINT_FUNCTION();
}

//================================================================================
// Main sketch
//================================================================================

const int pinLed = LED_BUILTIN;
unsigned long previousMillis = 0;

void setup() {
  // set LED to output
  pinMode(pinLed, OUTPUT);

  // pin change mask registers decide which pins are enabled as triggers
  PCMSK |= (1 << PCINT);

  // PCICR: Pin Change Interrupt Control Register - enables interrupt vectors
  PCICR |= (1 << PCIE);
}

void loop() {
  // turn led on LED for a few ms when received a valid IR signal
  if (millis() - previousMillis < 100)
    digitalWrite(pinLed, HIGH);
  else
    digitalWrite(pinLed, LOW);
}

void IREvent(uint8_t protocol, uint16_t address, uint32_t command) {
  // called when directly received an interrupt CHANGE.
  // do not use Serial inside, it can crash your program!
  previousMillis = millis();
}
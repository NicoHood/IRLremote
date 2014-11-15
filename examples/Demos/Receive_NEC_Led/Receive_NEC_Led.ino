/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive NEC Led
 Receives IR and lights the LED on a valid signal.

 This should demonstrate that you can use the IR sensor
 from almost every direction with almost no flash/ram.

 Flash usage (Uno):
 BlinkWithoutDelay:   1,006 bytes flash, 15 bytes ram
 IRL Receive NEC Led: 1,566 bytes flash, 26 bytes ram

 Ram usage (Uno):
 4byte attachInterrupt function
 4bytes lastTime
 for each protocol:
 4-6 bytes temporary work data
 1 byte temporary count
 = 8+(n*(5~7)) bytes ram used
 */

#include "IRLremote.h"

// see readme to choose the right pin (with an interrupt!) for your Arduino board
const int pinReceiveIR = digitalPinToInterrupt(2);

const int pinLed = 13;
unsigned long previousMillis = 0;

void setup() {
  // set LED to output
  pinMode(pinLed, OUTPUT);

  // choose NEC for this demonstration (most common protocol)
  IRLbegin<IR_NEC>(pinReceiveIR);
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
/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive NEC Led
 Receives IR and lights the LED on a valid signal.

 This should demonstrate that you can use the IR sensor
 from almost every direction with almost no ram/flash.

 BlinkWithoutDelay:   1,006 bytes flash, 15 bytes ram
 IRL Receive NEC Led: 1,580 bytes flash, 26 bytes ram

 Ram usage:
 4byte attachInterrupt function
 4bytes lastTime
 for each protocol:
 4-6 bytes temporary work data
 1 byte temporary count
 = 8+(n*5~7) bytes ram used
 */

#include "IRLremote.h"

// See readme to choose the right interrupt number
const int interruptIR = 0;

const int pinLed = 13;
unsigned long previousMillis = 0;

void setup() {
  pinMode(pinLed, OUTPUT);
  IRLbegin<IR_NEC>(interruptIR);
}

void loop() {
  // Turn led on for a few ms when received a valid IR signal
  if (millis() - previousMillis < 100)
    digitalWrite(pinLed, HIGH);
  else
    digitalWrite(pinLed, LOW);
}

void IREvent(uint8_t protocol, uint16_t address, uint32_t command) {
  // Called when received a valid IR signal
  // Do not use Serial inside, it can crash your Arduino!
  previousMillis = millis();
}
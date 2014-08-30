/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive Led Demo
 Receives IR signals and instantly calls an attached interrupt function.
 This should demonstrate that you can use the IR sensor from almost every direction.
 Blink example: 1,116 bytes flash, 11 bytes ram
 Receive Led Demo: 1,886 bytes flash, 49 bytes ram
 */

#include "IRLremote.h"

const int interruptIR = 0;

// choose your protocol
//IRLprotocolNEC IRprotocol;
IRLprotocolPanasonic IRprotocol;
//IRLprotocolAll IRprotocol;

const int pinLed = 13;
unsigned long previousMillis = 0;

void setup() {
  pinMode(pinLed, OUTPUT);

  // attach the interrupt function
  IRLremote.begin(interruptIR, IRprotocol, irEvent);
}

void loop() {
  // turn led on for a few ms when received a special IR command
  if (millis() - previousMillis < 100)
    digitalWrite(pinLed, HIGH);
  else
    digitalWrite(pinLed, LOW);
}

void irEvent(IR_Remote_Data_t IRData) {
  // Called when directly received correct IR Signal
  // Do not use Serial inside, it can crash your Arduino!
  if (IRData.command == 0x3D003805)
    previousMillis = millis();
}
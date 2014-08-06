/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRReceiveInterruptDemo
 Receives IR signals and instantly calls an attached interrupt function.
 This may fire more than one time if you press too long, add a debounce.
 Dont use Serial inside the interrupt!
 */

#include "IRLremote.h"

// select your protocol here
IRLremoteNEC IRLremote;
//IRLremotePanasonic IRLremote;

const int interruptIR = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  // attach the interrupt function
  IRLremote.begin(interruptIR, irEvent);
}

void loop() {
  // empty
}

void irEvent(IR_Remote_Data_t IRData) {
  // Called when directly received correct IR Signal
  // Do not use Serial inside, it can crash your Arduino.
  // This is only for debug
  Serial.println(IRData.address, HEX);
  Serial.println(IRData.command, HEX);
}


/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL ReceiveInterrupt Demo
 Receives IR signals and instantly calls an attached interrupt function.
 This may fire more than one time if you press too long, add a debounce.
 Dont use Serial inside the interrupt!
 */

#include "IRLremote.h"

const int interruptIR = 0;
IR_Remote_Data_t newestIRData;

// choose your protocol
IRLprotocolNEC IRprotocol;
//IRLprotocolPanasonic IRprotocol;
//IRLprotocolAll IRprotocol;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  // reset variables
  memset(&newestIRData, 0, sizeof(newestIRData));

  // attach the interrupt function
  IRLremote.begin(interruptIR, IRprotocol, irEvent);
}

void loop() {
  // temporary disable interrupts and print newest input
  cli();
  if (newestIRData.address || newestIRData.command) {
    Serial.print("Address:");
    Serial.println(newestIRData.address, HEX);
    Serial.print("Command:");
    Serial.println(newestIRData.command, HEX);
    // reset variables
    memset(&newestIRData, 0, sizeof(newestIRData));
  }
  sei();
}

void irEvent(IR_Remote_Data_t IRData) {
  // Called when directly received correct IR Signal
  // Do not use Serial inside, it can crash your Arduino!

  // Update the values to the newest valid input
  newestIRData = IRData;
}

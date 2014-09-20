/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Send Serial
 Sends IR signals on any pin. This uses Bitbanging.
 Write anything to the Serial port and hit enter to send Data.
 Turn interrupts off to get a better result if needed
 */

#include "IRLremote.h"

const int pinSendIR = 3;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");
}

void loop() {
  if (Serial.available()) {
    // discard all Serial bytes to avoid multiple sending
    delay(10);
    while (Serial.available())
      Serial.read();
      
    // send the data, no pin setting to OUTPUT needed
    Serial.println("Sending...");
    uint16_t address = 0xC686;
    uint32_t command = 0x7F80;
    IRLwrite(pinSendIR, address, command);
  }
}
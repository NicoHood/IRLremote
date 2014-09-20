/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Send Button
 Sends IR signals on any pin. This uses Bitbanging.
 Press the button to send data.
 Turn interrupts off to get a better result if needed
 */

#include "IRLremote.h"

const int pinSendIR = 3;
const int pinButton = 8;

void setup() {
  pinMode(pinButton, INPUT_PULLUP);
}

void loop() {
  if (!digitalRead(pinButton)) {
    // send the data, no pin setting to OUTPUT needed
    uint16_t address = 0xC686;
    uint32_t command = 0x7F80;
    IRLwrite(pinSendIR, address, command);

    // simple debounce
    delay(300);
  }
}

/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive_PinChangeInterrupt
 Receives IR signals with custom Pin Change Interrupt function.
 
 This is a PCINT implementation with an attach PCINT API only usable with Arduino Uno.
 I do not recommend to use it, because its very large. This was the first attempt
 to use PCINT and has now improved with the PCINT Basic example.
 This sketch is only here for legacy reasons.

 See second .ino file (next tab in IDE) for the pinchange implementation.
 This is perfectly usable if you dont want to use your normal interrupts for IRL.
 Keep in mind that this PCINT is not compatible with SoftSerial (at the moment).
 */

#include "IRLremote.h"

// function prototypes from 2nd .ino file
void PCattachInterrupt(uint8_t pin, void (*userFunc)(void), int mode);
void PCdetachInterrupt(uint8_t pin);

// choose your pin (must be on a PCINT pin though)
const int pinReceiveIR = 4;

// temporary variables to save latest IR input
uint8_t IRProtocol = 0;
uint16_t IRAddress = 0;
uint32_t IRCommand = 0;

void setup()
{
  // start serial debug output
  Serial.begin(115200);
  Serial.println("Startup");

  // choose your protocol here to reduce flash/ram/performance
  // see readme for more information
  PCattachInterrupt(pinReceiveIR, IRLinterrupt<IR_ALL>, CHANGE);
}

void loop() {
  // temporary disable interrupts and print newest input
  uint8_t oldSREG = SREG;
  cli();
  if (IRProtocol) {
    // print as much as you want in this function
    // see source to terminate what number is for each protocol
    Serial.println();
    Serial.print("Protocol:");
    Serial.println(IRProtocol);
    Serial.print("Address:");
    Serial.println(IRAddress, HEX);
    Serial.print("Command:");
    Serial.println(IRCommand, HEX);

    // reset variable to not read the same value twice
    IRProtocol = 0;
  }
  SREG = oldSREG;
}

void IREvent(uint8_t protocol, uint16_t address, uint32_t command) {
  // called when directly received an interrupt CHANGE.
  // do not use Serial inside, it can crash your program!

  // update the values to the newest valid input
  IRProtocol = protocol;
  IRAddress = address;
  IRCommand = command;
}

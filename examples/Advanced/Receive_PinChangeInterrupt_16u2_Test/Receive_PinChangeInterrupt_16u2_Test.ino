/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive_PinChangeInterrupt_16u2_Test
 Receives IR signals with custom Pin Change Interrupt function.

 This sketch is especially made for the 16u2 combined with the HoodLoader2.
 It lights the TX Led on any valid signal and sends on a special input.
 The Receiver is connected to PB1 (SCLK).
 The Transmitter is connected to PB2 (MOSI).

 This is the very basic PCINT version to use almost any pin for the IRLremote.
 All you need to do is look up the ports in the datasheet.
 Keep in mind that this PCINT is not compatible with SoftSerial (at the moment).
 */

#include "IRLremote.h"

//================================================================================
// PCINT functions
//================================================================================

// PCINT definitions, edit and see datasheets for more information
#define PCINT_PIN 3
#define PCMSK *digitalPinToPCMSK(PCINT_PIN)
#define PCINT digitalPinToPCMSKbit(PCINT_PIN)
#define PCIE  digitalPinToPCICRbit(PCINT_PIN)
#define PCPIN *portInputRegister(digitalPinToPort(PCINT_PIN))
#define PCINT_vect PCINT0_vect // TODO map somehow to the pin
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

// workaround for undefined USBCON
//	General interrupt
ISR(USB_GEN_vect)
{
  uint8_t udint = UDINT;
  UDINT = 0;
  return;
}

// variable to save the last update
unsigned long previousMillis = 0;

// choose any pin to send IR signals
const int pinSendIR = MOSI; // pin2 on 16u2

// variable to save a sending trigger
bool sendNow = false;

void setup() {
  // set LED to output
  pinMode(LED_BUILTIN_TX, OUTPUT);
  pinMode(LED_BUILTIN_RX, OUTPUT);

  // attach the new PinChangeInterrupt function above
  attachPinChangeInterrupt();
}

void loop() {
  // turn led on LED for a few ms when received a valid IR signal
  if (millis() - previousMillis < 100)
    digitalWrite(LED_BUILTIN_TX, LOW);
  else
    digitalWrite(LED_BUILTIN_TX, HIGH);

  // check if the input was a specific signal and send another signal out
  if (sendNow) {
    // turn on led
    digitalWrite(LED_BUILTIN_RX, LOW);

    // send the data, no pin setting to OUTPUT needed
    uint16_t address = 0x6361;
    uint32_t command = 0xFE01;

    IRLwrite<IR_NEC>(pinSendIR, address, command);

    // turn off led and reset trigger
    digitalWrite(LED_BUILTIN_RX, HIGH);
    sendNow = false;
  }
}

void IREvent(uint8_t protocol, uint16_t address, uint32_t command) {
  // called when directly received an interrupt CHANGE.
  // do not use Serial inside, it can crash your program!
  previousMillis = millis();

  // set the trigger if input signal is the choosen
  if (protocol == IR_PANASONIC && address == 0x2002 && command == 0x813D1CA0)
    sendNow = true;
}

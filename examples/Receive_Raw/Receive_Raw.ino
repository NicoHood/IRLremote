/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Receive_Raw
 Receives IR signals and prints raw values to the Serial.
 Each dump should end with a timeout.
 
 This also demonstrates how to implement your own decoding functions.
 See PinInterrupt or PinChangeInterrupt examples for basic usage.
 */

#include "IRLremote.h"

// choose a valid PinInterrupt pin of your Arduino board
#define pinIR 2

// variables to record raw values
#define IR_RAW_TIMEOUT 0xFFFF // 65535, max timeout
#define IR_RAW_BUFFER_SIZE 100 // 0-255
uint16_t buffer[IR_RAW_BUFFER_SIZE];
uint8_t count = 0;

void setup() {
  // start serial debug output
  Serial.begin(115200);
  Serial.println("Startup");

  // set protocol to user to use the decode function below
  attachInterrupt(digitalPinToInterrupt(pinIR), IRLinterrupt<IR_USER>, CHANGE);
}

void loop() {
  // check if raw buff is full or timed out.
  // keep in mind that the defined timeout above might be in the output (by default 50000)
  // so you dont get confused where the number is from.
  bool newInput = RAWIRLavailable();
  if (newInput) {
    // print a mark
    Serial.println("==========");

    // go through the whole buffer and print values
    for (uint8_t i = 0; i < count; i++) {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(buffer[i], DEC);
    }

    // each buffer should end with the timeout value
    if (count == IR_RAW_BUFFER_SIZE)
      Serial.println("Buffer was full!");

    // reset for new reading
    count = 0;
  }
}

void decodeIR(const uint16_t duration) {
  // called when directly received an interrupt CHANGE.
  // do not use Serial inside, it can crash your program!

  // block until data is read
  if (RAWIRLavailable()) return;

  // save value and increase count
  buffer[count++] = duration;
}

bool RAWIRLavailable(void) {
  // disable interrupts when checking for new input
  uint8_t oldSREG = SREG;
  cli();

  // check if we have data
  if (count) {
    // check if buffer is full
    if (count == IR_RAW_BUFFER_SIZE) {
      SREG = oldSREG;
      return true;
    }

    // check if last reading was a timeout.
    // no problem if count==0 because the if above prevents this
    if (buffer[count - 1] >= IR_RAW_TIMEOUT) {
      SREG = oldSREG;
      return true;
    }

    // check if reading timed out and save value.
    // saving is needed to abord the check above next time.
    uint32_t duration = micros() - IRL_LastTime;
    if (duration >= IR_RAW_TIMEOUT) {
      buffer[count++] = IR_RAW_TIMEOUT;
      SREG = oldSREG;
      return true;
    }
  }

  // continue, we can still save into the buffer or the buffer is empty
  SREG = oldSREG;
  return false;
}

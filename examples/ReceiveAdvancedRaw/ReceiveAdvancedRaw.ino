/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL ReceiveAdvancedRaw
 Receives IR signals and prints raw values to the Serial.
 This also demonstrates how to implement your own decoding functions.
 */

#include "IRLremote.h"

// See readme to choose the right interrupt number
const int interruptIR = 0;

// variables to record raw values
#define IR_RAW_TIMEOUT 50000
#define IR_RAW_BUFFER_SIZE 100
uint32_t buffer[IR_RAW_BUFFER_SIZE];
uint8_t count = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  // set protocol to user to use the decode function below
  IRLbegin<IR_USER>(interruptIR);
}

void loop() {
  // check if raw buff is full or timed out
  uint8_t irBytes = RAWIRLavailable();
  if (irBytes) {
    // print a mark
    Serial.println("==========");

    // go through the whole buffer and print values
    for (int i = 0; i < irBytes; i++) {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(buffer[i], DEC);
    }

    // reset for new reading
    count = 0;
  }
}

void decodeIR(const uint32_t duration) {
  // Called when directly received and interrupt CHANGE
  // Do not use Serial inside, it can crash your Arduino!

  // block until data is read
  if (RAWIRLavailable()) return;

  // save value and increase count
  buffer[count++] = duration;

  // save last time for timeout check in the buff too
  if (count != IR_RAW_BUFFER_SIZE)
    buffer[count] = micros();
}

uint8_t RAWIRLavailable(void) {
  // disable interrupts when checking for new input
  uint8_t oldSREG = SREG;
  cli();

  // check if we have data
  if (count) {
    // check if buffer is full
    if (count == IR_RAW_BUFFER_SIZE) {
      SREG = oldSREG;
      return count;
    }

    // check if last reading was a timeout
    // no problem if count==0 because the if above prevents this
    // anyways then it will just return 0
    if (buffer[count - 1] > IR_RAW_TIMEOUT) {
      SREG = oldSREG;
      return count;
    }

    // check if reading timed out and save value
    unsigned long duration = micros() - buffer[count];
    if (duration > IR_RAW_TIMEOUT) {
      buffer[count++] = duration;
      SREG = oldSREG;
      return count;
    }
  }

  SREG = oldSREG;
  return 0;
}

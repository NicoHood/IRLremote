/*
  CIRLremote3.cpp - library - description
 Copyright (c) 2014 NicoHood.  All right reserved.
 */

#include "IRLremote3.h"

CIRLremote IRLremote;

// static variables
uint8_t CIRLremote::mLowBits [IR_BLOCKS];
uint8_t CIRLremote::mHighBits[IR_BLOCKS];
unsigned long CIRLremote::mLastTime=0;
uint8_t CIRLremote::mCount=0;
uint32_t CIRLremote::mCommand=0;
void (*CIRLremote::user_onReceive)(uint32_t);


CIRLremote::CIRLremote(void){
	// empty
}


void CIRLremote::enable(uint8_t interrupt, void (*function)(uint32_t)){
  mInterrupt=interrupt;
  user_onReceive = function;
  attachInterrupt(mInterrupt, decodeIR, CHANGE);
}


void CIRLremote::disable(void){
  detachInterrupt(mInterrupt);
}

// inline
//uint32_t CIRLremote::read(void){
//  // deactivate interrupt, save and erease command
//  cli();
//  uint32_t values=mCommand;
//  mCommand=0;
//  sei();
//  return values;
//}


void CIRLremote::decodeIR(void){ //called interrupt CHANGE

	// if you do not want to get the latest, updated input use this
	// you might want to remove the earising in the read function
	// and add a resume function which does this for you then.
	//if(mCommand) return;

  //save the duration between the last reading
  unsigned long time = micros();
  unsigned long duration = time - mLastTime;
  mLastTime = time;


  // if timeout(start next value)
  if (duration >=IR_TIMEOUT){
    mCount=0;
  }

  // check Lead (needs a timeout or a correct signal)
  else if (mCount==0){
    // lead is okay
    if(duration>(IR_SPACE_HOLDING+IR_LEAD)/2) mCount++;
    // wrong lead
    else mCount=0;
  }

  //check Space/Space Holding
  else if(mCount==1){ 
#if (IR_SPACE != IR_SPACE_HOLDING)
    // normal Space
    if (duration>(IR_SPACE+IR_SPACE_HOLDING)/2){
      // next reading
      mCount++;
    }

    // Button holding
    else if(duration>(IR_HIGH_1+IR_SPACE_HOLDING)/2){ 
      mCommand=-1;
      mCount=0;
    }
#else //no Space Holding
    // normal Space
    if (duration>(IR_SPACE+IR_HIGH_1)/2) mCount++; // next reading
#endif
    // wrong space
    else mCount=0;
  }

  // High pulses (odd numbers)
  else if(mCount%2==1){ 
    // get number of the High Bits
    // minus one for the lead
    uint8_t length=(mCount/2)-1;  

    // write logical 1
    if(duration>(IR_HIGH_0+IR_HIGH_1)/2) mHighBits[length/8]|=(0x80>>(length%8));
    // write logical 0
    else mHighBits[length/8]&=~(0x80>>(length%8));
    // next reading
    mCount++;
  }

  // Low pulses (even numbers)
  else{ 
    // You dont really need to check them for errors.
    // But you might miss some wrong values
    // Checking takes more operations but is safer.
    // We want maximum recognition so we leave this out here.
    // also we have the inverse or the XOR to check the data.

    // write low bits
    //if(duration>(IR_LOW_0+IR_LOW_1)/2);
    //else;

    //check for error
    //if(duration>(IR_HIGH_0+IR_HIGH_1)/2) mCount=0;
    //else
    mCount++;
  }

  // check last input
  if(mCount>=IR_LENGTH){
    /*
     //Debug output
     for(int i=0; i<IR_BLOCKS;i++){
     for(int j=0; j<8;j++){
     Serial.print((mHighBits[i]>>(7-j))&0x01);
     }
     Serial.println();
     }
     Serial.println();
     */

    //write command based on each Protocol
#ifdef PANASONIC
    // Errorcorrection for Panasonic with XOR
    // Address left out, we dont need that (0,1)
    if(uint8_t(mHighBits[2]^mHighBits[3]^mHighBits[4]) == mHighBits[5]){
      //in my case [2] has a fix value
      uint32_t command=(mHighBits[3]<<8)+mHighBits[4];
	  user_onReceive(command);
    }
#endif

#ifdef NEC
    // You can also check the Address, but i wont do that.
    // In some other Nec Protocols the Address has an inverse or not
    if(uint8_t((mHighBits[2]^(~mHighBits[3]))) ==0){
      // Errorcorrection for the Command is the inverse
      uint32_t command=mHighBits[2];
	  user_onReceive(command);
    }
#endif

    mCount=0;
  }
}

//================================================================================
//RAW
//================================================================================

void CIRLremote::enableRAW(uint8_t interrupt){
  mInterrupt=interrupt;
  attachInterrupt(mInterrupt, decodeRAW, CHANGE);
}

void CIRLremote::decodeRAW(void){
  //save the duration between the last reading
  unsigned long time = micros();
  unsigned long duration = time - mLastTime;
  mLastTime = time;
  mCount++;

  Serial.println(duration);
  //if timeout print last length + mark
  if (duration >=30000){
    Serial.print(mCount);
    Serial.println("<==");
    mCount=0;
  } 
}


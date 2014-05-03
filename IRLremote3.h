/*
CIRLremote3.h - library - description
Copyright (c) 2014 NicoHood.  All right reserved.
*/

#ifndef IRLremote3_h
#define IRLremote3_h

#include <Arduino.h>

// Select your protocol here
// information about protocols:
// http://www.hifi-remote.com/johnsfine/DecodeIR.html

//#define NEC
#define PANASONIC 

//NEC
//IRP notation: {38.4k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,-78,(16,-4,1,-173)*) 
#if defined(NEC)
#define IR_PULSE 564
#define IR_BLOCKS 4
#define IR_LEAD IR_PULSE*16
#define IR_SPACE IR_PULSE*8
#define IR_SPACE_HOLDING IR_PULSE*4
#define IR_LOW_0 IR_PULSE*1
#define IR_LOW_1 IR_PULSE*1
#define IR_HIGH_0 IR_PULSE*1
#define IR_HIGH_1 IR_PULSE*3
#define IR_LENGTH 2 + IR_BLOCKS*8*2 //2 for lead&space, each block has 8bits: low and high
#define IR_TIMEOUT IR_PULSE*173/2
#endif

//PANASONIC
//IRP notation: {37k,432}<1,-1|1,-3>(8,-4,3:8,1:8,D:8,S:8,F:8,(D^S^F):8,1,-173)+ 
#if defined(PANASONIC)
#define IR_PULSE 432
#define IR_BLOCKS 6
#define IR_LEAD IR_PULSE*8
#define IR_SPACE IR_PULSE*4
#define IR_SPACE_HOLDING IR_SPACE //no holding
#define IR_LOW_0 IR_PULSE*1
#define IR_LOW_1 IR_PULSE*1
#define IR_HIGH_0 IR_PULSE*1
#define IR_HIGH_1 IR_PULSE*3
#define IR_LENGTH 2 + IR_BLOCKS*8*2 //2 for lead&space, each block has 8bits: low and high
#define IR_TIMEOUT IR_PULSE*173/2
#endif

class CIRLremote{
public:
	CIRLremote(void);
	void enable(uint8_t interrupt, void (*)(uint32_t)=defaultReceive);
	void enableRAW(uint8_t interrupt);
	void disable(void);

	inline uint32_t read(void){
		// deactivate interrupt, save and erease command
		cli();
		uint32_t values=mCommand;
		mCommand=0;
		sei();
		return values;
	};

	inline uint8_t available(void){
		if(mCommand) return true;
		else return false;
	};

private:
	static void decodeIR(void);
	static void decodeRAW(void);
	uint8_t mInterrupt;
	static void (*user_onReceive)(uint32_t);
	static inline void defaultReceive(uint32_t command){
		mCommand=command;};

	// variables to save the last reading and count the readings
	static unsigned long  mLastTime;
	static uint8_t mCount;

	// stores the logic of the low and high pulses
	static uint8_t mLowBits [IR_BLOCKS];
	static uint8_t mHighBits[IR_BLOCKS];

	// Command stored
	static uint32_t mCommand;

};

extern CIRLremote IRLremote;

#endif



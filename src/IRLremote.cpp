
#include "IRLremote.h"


//TODO move
uint8_t CIRLNec::countNec = 0;
uint8_t CIRLNec::dataNec[NEC_BLOCKS] = { 0 };


uint8_t CIRLData::IRLProtocol = IR_NO_PROTOCOL;
uint32_t CIRLData::IRLLastTime = 0;
uint32_t CIRLData::IRLLastEvent = 0;

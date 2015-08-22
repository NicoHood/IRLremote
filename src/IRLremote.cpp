
#include "IRLremote.h"

uint8_t IRLProtocol = IR_NO_PROTOCOL;
uint32_t IRLLastTime = 0;
uint32_t IRLLastEvent = 0;

//TODO move
uint8_t CIRLNec::countNec = 0;
uint8_t CIRLNec::dataNec[NEC_BLOCKS] = { 0 };




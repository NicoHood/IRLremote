///*
//Copyright (c) 2014 NicoHood
//See the readme for credit to other people.
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
//*/
//
//#ifndef IRLPROTOCOLRAW_H
//#define IRLPROTOCOLRAW_H
//
//// include the main remote
//#include <CIRLremote.h>
//
//#define IR_RAW_TIMEOUT 50000
//#define IR_RAW_BUFFER_SIZE 100
//
//class IRLprotocolRaw : public CIRLprotocol{
//public:
//	IRLprotocolRaw(void){ }
//
//	// virtual functions that needs to be implemented:
//	bool decodeIR(unsigned long duration);
//	void reset(void);
//
//	// new functions for the user to access the raw buffer
//	uint8_t available(void);
//	uint32_t buffer[IR_RAW_BUFFER_SIZE];
//
//private:
//	uint8_t mCount;
//};
//
//#endif

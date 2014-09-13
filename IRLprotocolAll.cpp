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
//#include <IRLprotocolAll.h>
//
//void IRLprotocolAll::reset(void){
//	_IRLprotocolNEC.reset();
//	_IRLprotocolPanasonic.reset();
//}
//
//bool IRLprotocolAll::decodeIR(unsigned long duration){
//	// go through all Protocols and stop if a valid input was found
//	// this shouldnt take too long, otherwise it will miss values
//	if (_IRLprotocolNEC.decodeIR(duration))
//		IRData = _IRLprotocolNEC.IRData;
//	else if (_IRLprotocolPanasonic.decodeIR(duration))
//		IRData = _IRLprotocolPanasonic.IRData;
//	else
//		return false;
//
//	// for any new input: reset all protocols and return true
//	reset();
//	return true;
//}
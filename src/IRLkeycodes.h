/*
Copyright (c) 2014-2015 NicoHood
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// include guard
#pragma once

//================================================================================
// Protek 9700 Series
//================================================================================

#define IRL_PROTEK_PROTOCOL		IR_NEC_EXTENDED
#define IRL_PROTEK_REPEAT		IR_NEC_REPEAT
#define IRL_PROTEK_ADDRESS		0x2222

#define IRL_PROTEK_POWER		0xFD02
#define IRL_PROTEK_MUTE			0xE916

#define IRL_PROTEK_SCREEN		0xD42B
#define IRL_PROTEK_SATELLITE	0xD32C
#define IRL_PROTEK_TV_RADIO		0xD22D
#define IRL_PROTEK_TV_MUSIC		0xD12E

#define IRL_PROTEK_1			0xEC13
#define IRL_PROTEK_2			0xEB14
#define IRL_PROTEK_3			0xEA15
#define IRL_PROTEK_4			0xE817
#define IRL_PROTEK_5			0xE718
#define IRL_PROTEK_6			0xE619
#define IRL_PROTEK_7			0xE41B
#define IRL_PROTEK_8			0xE31C
#define IRL_PROTEK_9			0xE21D
#define IRL_PROTEK_BACK			0xF10E
#define IRL_PROTEK_0			0xF30C
#define IRL_PROTEK_FAVORITE		0xFB04

#define IRL_PROTEK_VOL_UP		0xDF20
#define IRL_PROTEK_VOL_DOWN		0xDE21
#define IRL_PROTEK_EPG			0xF708
#define IRL_PROTEK_INFO			0xED12
#define IRL_PROTEK_CHANNEL_UP	0xDD22
#define IRL_PROTEK_CHANNEL_DOWN	0xDC23

#define IRL_PROTEK_UP			0xF609
#define IRL_PROTEK_DOWN			0xF807
#define IRL_PROTEK_LEFT			0xF50A
#define IRL_PROTEK_RIGHT		0xF906
#define IRL_PROTEK_OK			0xEE11

#define IRL_PROTEK_EXIT			0xEF10
#define IRL_PROTEK_MENU			0xF00F

#define IRL_PROTEK_I_II			0xF20D
#define IRL_PROTEK_TELETEXT		0xE01F
#define IRL_PROTEK_SUBTITLE		0xFE01
#define IRL_PROTEK_ADD			0xD02F

#define IRL_PROTEK_RED			0xD827
#define IRL_PROTEK_GREEN		0xD728
#define IRL_PROTEK_YELLOW		0xD629
#define IRL_PROTEK_BLUE			0xD52A

#define IRL_PROTEK_PREV			0xCF30
#define IRL_PROTEK_PLAY			0xCE31
#define IRL_PROTEK_STOP			0xCD32
#define IRL_PROTEK_NEXT			0xCC33
#define IRL_PROTEK_USB			0xCB34
#define IRL_PROTEK_PAUSE		0xCA35
#define IRL_PROTEK_REC			0xC936
#define IRL_PROTEK_LIVE			0xC837

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
// Protocol Definitions
//================================================================================

//NEC
//IRP notation: {38.4k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,-78,(16,-4,1,-173)*)
// Lead + Space logic
#define NEC_HZ				38400
#define NEC_PULSE			564UL
#define NEC_BLOCKS			4
#define NEC_ADDRESS_LENGTH	16
#define NEC_COMMAND_LENGTH	16
#define NEC_LENGTH			(2 + NEC_BLOCKS * 8 * 2) // 2 for lead + space, each block has 8bits: mark and space
#define NEC_TIMEOUT			(NEC_PULSE * 78UL)
#define NEC_TIMEOUT_HOLDING (NEC_PULSE * 173UL)
#define NEC_TIMEOUT_REPEAT	(NEC_TIMEOUT + NEC_MARK_LEAD + NEC_SPACE_LEAD \
							+ NEC_MARK_ZERO * 16 + NEC_MARK_ONE * 16 \
							+ NEC_SPACE_ZERO * 16 + NEC_SPACE_ONE * 16)
#define NEC_MARK_LEAD		(NEC_PULSE * 16)
#define NEC_SPACE_LEAD		(NEC_PULSE * 8)
#define NEC_SPACE_HOLDING	(NEC_PULSE * 4)
#define NEC_MARK_ZERO		(NEC_PULSE * 1)
#define NEC_MARK_ONE		(NEC_PULSE * 1)
#define NEC_SPACE_ZERO		(NEC_PULSE * 1)
#define NEC_SPACE_ONE		(NEC_PULSE * 3)

//PANASONIC
//IRP notation: {37k,432}<1,-1|1,-3>(8,-4,3:8,1:8,D:8,S:8,F:8,(D^S^F):8,1,-173)+
// Lead + Space logic
#define PANASONIC_HZ 37000
#define PANASONIC_PULSE 432UL
#define PANASONIC_BLOCKS 6
#define PANASONIC_ADDRESS_LENGTH 16
#define PANASONIC_COMMAND_LENGTH 32
#define PANASONIC_LENGTH 2 + PANASONIC_BLOCKS*8*2 // 2 for lead + space, each block has 8bits: mark and space
#define PANASONIC_TIMEOUT PANASONIC_PULSE*173
#define PANASONIC_MARK_LEAD PANASONIC_PULSE*8
#define PANASONIC_SPACE_LEAD PANASONIC_PULSE*4
#define PANASONIC_SPACE_HOLDING 0 // no holding function in this protocol
#define PANASONIC_MARK_ZERO PANASONIC_PULSE*1
#define PANASONIC_MARK_ONE PANASONIC_PULSE*1
#define PANASONIC_SPACE_ZERO PANASONIC_PULSE*1
#define PANASONIC_SPACE_ONE PANASONIC_PULSE*3

/*
Panasonic pulse demonstration:

*---------|                        |------------|   |---------|   |---|   ... -|   |---------
*         |                        |            |   |         |   |   |   ...  |   |
*         |                        |            |   |         |   |   |   ...  |   |
*         |------------------------|            |---|         |---|   |-  ...  |---|
*         |       Lead Mark        | Lead Space |  Logical 1  | Log 0 |  Data  |End|

*/

//SONY 8, 12, 15, 20
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:8,^22200)
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:7,D:5,^45m)+ 
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:7,D:8,^45m)+ 
//IRP notation: {40k,600}<1,-1|2,-1>(4,-1,F:7,D:5,S:8,^45m) +
// Lead + Mark logic
#define SONY_HZ 40000
#define SONY_PULSE 600UL
#define SONY_BLOCKS_8 1
#define SONY_BLOCKS_12 2
#define SONY_BLOCKS_15 2
#define SONY_BLOCKS_20 3
#define SONY_ADDRESS_LENGTH_8 0
#define SONY_ADDRESS_LENGTH_12 5
#define SONY_ADDRESS_LENGTH_15 8
#define SONY_ADDRESS_LENGTH_20 13
#define SONY_COMMAND_LENGTH_8 8
#define SONY_COMMAND_LENGTH_12 7
#define SONY_COMMAND_LENGTH_15 7
#define SONY_COMMAND_LENGTH_20 7
#define SONY_LENGTH_8 2 + (8-1)*2 // 2 for lead + space, -1 for mark end, 8 bit
#define SONY_LENGTH_12 2 + (7+5-1)*2 // 2 for lead + space, -1 for mark end, 12 bit
#define SONY_LENGTH_15 2 + (7+8-1)*2 // 2 for lead + space, -1 for mark end, 15 bit
#define SONY_LENGTH_20 2 + (7+5+8-1)*2 // 2 for lead + space, -1 for mark end, 20 bit
#define SONY_TIMEOUT_8 22200
#define SONY_TIMEOUT 45000 // 12, 15, 20 have the same timeout
#define SONY_MARK_LEAD SONY_PULSE*4
#define SONY_SPACE_LEAD SONY_PULSE*1
#define SONY_SPACE_HOLDING 0 // no holding function in this protocol
#define SONY_MARK_ZERO SONY_PULSE*1
#define SONY_MARK_ONE SONY_PULSE*2
#define SONY_SPACE_ZERO SONY_PULSE*1
#define SONY_SPACE_ONE SONY_PULSE*1
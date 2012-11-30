/*
 * clock.h
 *
 *  Created on: Nov 29, 2012
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#define FEE_FREQ		48000000
#define XTAL_FREQ		8000000

typedef uint8_t clk_mode_t; enum {
	FEI, //FLL Engaged Internal - 24MHz (from FLL using 32khz internal)
	FEE,
	FBI,
	FBE, //FLL Bypassed External - 8MHz external
	PEE, //PLL Engaged External
	PBE, //PLL Bypassed External
	BLPI,
	BLPE,
	STOP,
	
};


void init_clock();
void clock_mode(clk_mode_t clk);
clk_mode_t get_clock_state();


#endif /* CLOCK_H_ */

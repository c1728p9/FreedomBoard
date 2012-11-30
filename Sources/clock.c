/*
 * clock.c
 *
 *  Created on: Nov 29, 2012
 */

#include "derivative.h"

#include "clock.h"
#include "essentials.h"



static clk_mode_t get_next_clock_state(clk_mode_t cur, clk_mode_t tar);
static void set_clock_state(clk_mode_t state);


void init_clock() {
	/* MCG->C2: LOCRE0=0,??=0,RANGE0=2,HGO0=0,EREFS0=1,LP=0,IRCS=0 */
	//Crystal is a low power crystal (HGO0 = 0)
	MCG_C2 = MCG_C2_RANGE0(2) |	//High frequency, low power
		MCG_C2_EREFS0_MASK; //External crystal (not clock input)
	
	
	// Only change if PLL is not enabled
	if ( 0 == (MCG_S & MCG_S_PLLST_MASK) ) {
		MCG_C5 = (MCG_C5 
				& ~MCG_C5_PRDIV0_MASK)
				| MCG_C5_PRDIV0(1); // 3 = divide by 2 (8MHz -> 4MHz) 
		
		MCG_C6 = (MCG_C6
				& ~MCG_C6_VDIV0_MASK)
				| MCG_C6_VDIV0(0);// Multiply by 24 ( 4MHz * 24 = 96MHz)
		
	}
	
	// Switch to fastest mode
    clock_mode(PEE);    
    if (get_clock_state() != PEE)
        error();
		
}


void clock_mode(clk_mode_t target_state) {
	
	clk_mode_t current_state = get_clock_state();
	while (current_state != target_state) {
		clk_mode_t next_state = get_next_clock_state(current_state, target_state);
		set_clock_state(next_state);
		current_state = get_clock_state();
	}
	
}



clk_mode_t get_clock_state() {
	enum {
		CLKST_FLL,
		CLKST_INT,
		CLKST_EXT,
		CLKST_PLL
	};
	
	uint8_t low_power = (MCG_C2 & MCG_C2_LP_MASK) >> MCG_C2_LP_SHIFT;
	uint8_t clkst = (MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT;
	uint8_t pll_nfll = (MCG_S & MCG_S_PLLST_MASK) >> MCG_S_PLLST_SHIFT;
	uint8_t fll_int_nExt = (MCG_S & MCG_S_IREFST_MASK) >> MCG_S_IREFST_SHIFT;
	int state;
	
	if (low_power) {
		if (CLKST_EXT == clkst) {
			state = BLPE;
		} else {
			state = BLPI;
		}
	} else /* normal power */ {
		if (pll_nfll) {
			if (CLKST_PLL == clkst) {
				state = PEE; //PLL is selected as main clock
			} else {
				state = PBE; // PLL is bypassed
			}
		} else /*fll*/ {
			if (CLKST_FLL == clkst) {
				if (fll_int_nExt) {
					state = FEI; //FLL internal
				} else {
					state = FEE; //FLL external
				}
				
			} else if (CLKST_EXT == clkst) {
				state = FBE; //FLL bypassed external
			} else {
				state = FBI; //FLL bypassed internal
			}
		}
	}
	
	return state;
}

static clk_mode_t get_next_clock_state(clk_mode_t cur, clk_mode_t tar) {
	clk_mode_t next_mode = tar;
	if (cur == tar) return tar;
	
	switch (cur) {
	case FEI:
	case FEE:
	case FBI:
	case FBE:
		switch (tar) {
		case FEI:
		case FEE:
		case FBI:
		case FBE:
			next_mode = tar;
			break;
		case PEE:
		case PBE:
			if (FBE == cur) {
				next_mode = PBE;
			} else {
				next_mode = FBE;
			}
			break;
		case BLPI:
			if (FBI == cur) {
				next_mode = BLPI;
			} else {
				next_mode = FBI;
			}
			break;
		case BLPE:
			if (FBE == cur) {
				next_mode = BLPE;
			} else {
				next_mode = FBE;
			}
			break;
		default:
			//should never occur
			break;
		}
		break;
	case PEE:
		next_mode = PBE;
		break;
	case PBE:
		if (PEE == tar) {
			next_mode = PEE;
		} else if (BLPE == tar) {
			next_mode = BLPE;
		} else {
			next_mode = FBE;
		}
		break;
	case BLPI:
		next_mode = FBI;
		break;
	case BLPE://
		if (PBE == tar || PEE == tar) {
			next_mode = PBE;
		} else {
			next_mode = FBE;
		}
		break;
	default:
		//should never occur
		break;
	}
	return next_mode;
}

static void set_clock_state(clk_mode_t state) {
	switch (state) {
	case FEI:
		// C1[CLKS] bits are written to 00
		// C1[IREFS] bit is written to 1
		// C6[PLLS] bit is written to 0
		MCG_C4 = (MCG_C4 
			& ~MCG_C4_DRST_DRS_MASK
			& ~MCG_C4_DMX32_MASK)
			| MCG_C4_DRST_DRS(0)
			| (0 << MCG_C4_DRST_DRS_SHIFT);
		MCG_C1 = (MCG_C1 
			& ~MCG_C1_CLKS_MASK
			& ~MCG_C1_IREFS_MASK)
			| MCG_C1_CLKS(0)
			| (1 << MCG_C1_IREFS_SHIFT);
		MCG_C6 = (MCG_C6
				& MCG_C6_PLLS_MASK)
				| (0 << MCG_C6_PLLS_SHIFT);
		
		//check irefst
		//check clkst
		while ((MCG_S & MCG_S_IREFST_MASK) != (1 << MCG_S_IREFST_SHIFT));
		while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(0));
		
		break;
		
	case FEE:
		// C1[CLKS] bits are written to 00
		// C1[IREFS] bit is written to 0
		// C1[FRDIV] must be written to divide external reference clock to be within the range of 31.25 kHz to 39.0625 kHz
		// C6[PLLS] bit is written to 0
		MCG_C1 = (MCG_C1 
			& ~MCG_C1_CLKS_MASK
			& ~MCG_C1_IREFS_MASK
			& ~MCG_C1_FRDIV_MASK)
			| MCG_C1_CLKS(0)
			| (0 << MCG_C1_IREFS_SHIFT)
			| MCG_C1_FRDIV(4); //Divide 8MHz external down to 31.25 KHz (div by 256)
		MCG_C6 =(MCG_C6
				& ~MCG_C6_PLLS_MASK)
				| (0 << MCG_C6_PLLS_SHIFT);
		
		// Loop until S[OSCINIT0] is 1, indicating the crystal selected by the C2[EREFS0] bit has been initialized.
		// Loop until S[IREFST] is 0, indicating the external reference clock is the current source for the reference clock.
		// Loop until S[CLKST] are 2'b00, indicating that the output of the FLL is selected	to feed MCGOUTCLK.
		while ((MCG_S & MCG_S_OSCINIT0_MASK) != (1 << MCG_S_OSCINIT0_SHIFT)); 
		while ((MCG_S & MCG_S_IREFST_MASK) != (0 << MCG_S_IREFST_SHIFT)); 
		while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(0));
		break;
		
	case FBI:
		// C1[CLKS] bits are written to 01
		// C1[IREFS] bit is written to 1
		// C6[PLLS] is written to 0
		// C2[LP] is written to 0
		MCG_C1 = (MCG_C1 
			& ~MCG_C1_CLKS_MASK
			& ~MCG_C1_IREFS_MASK)
			| MCG_C1_CLKS(1)
			| (0 << MCG_C1_IREFS_SHIFT);
		MCG_C6 =(MCG_C6
				& ~MCG_C6_PLLS_MASK)
				| (0 << MCG_C6_PLLS_SHIFT);
		MCG_C2 = (MCG_C2
				& ~MCG_C2_LP_MASK)
				| (0 << MCG_C2_LP_SHIFT);
		
		
		// Loop until S[IREFST] is 1, indicating the internal reference clock has been selected as the reference clock source.
		// Loop until S[CLKST] are 2'b01, indicating that the internal reference clock is selected to feed MCGOUTCLK.
		while ((MCG_S & MCG_S_IREFST_MASK) != (1 << MCG_S_IREFST_SHIFT)); 
		while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(1));
		break;
		
	case FBE:
		// C1[CLKS] bits are written to 10
		// C1[IREFS] bit is written to 0
		// C1[FRDIV] must be written to divide external reference clock to be within the range of 31.25 kHz to 39.0625 kHz.
		// C6[PLLS] bit is written to 0
		// C2[LP] is written to 0
		MCG_C1 = (MCG_C1 
			& ~MCG_C1_CLKS_MASK
			& ~MCG_C1_IREFS_MASK
			& ~MCG_C1_FRDIV_MASK)
			| MCG_C1_CLKS(2)
			| (0 << MCG_C1_IREFS_SHIFT)
			| MCG_C1_FRDIV(4); //Divide 8MHz external down to 31.25 KHz (div by 256)
		MCG_C6 =(MCG_C6
				& ~MCG_C6_PLLS_MASK)
				| (0 << MCG_C6_PLLS_SHIFT);
		MCG_C2 = (MCG_C2
				& ~MCG_C2_LP_MASK)
				| (0 << MCG_C2_LP_SHIFT);
		
		//Loop until S[OSCINIT0] is 1, indicating the crystal selected by C2[EREFS0] has been initialized.
		//Loop until S[IREFST] is 0, indicating the external reference is the current source for the reference clock.
		//Loop until S[CLKST] is 2'b10, indicating that the external reference clock is selected to feed MCGOUTCLK.
		while ((MCG_S & MCG_S_OSCINIT0_MASK) != (1 << MCG_S_OSCINIT0_SHIFT)); 
		while ((MCG_S & MCG_S_IREFST_MASK) != (0 << MCG_S_IREFST_SHIFT)); 
		while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(2));
		break;
		
	case PEE:
		//Divide main clock by 2 (96 -> 48), divide flash by an additional 2 (48 -> 24)
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1) | SIM_CLKDIV1_OUTDIV4(1);
		
		// C1[CLKS] bits are written to 00
		// C1[IREFS] bit is written to 0
		// C6[PLLS] bit is written to 1
		MCG_C1 = (MCG_C1 
			& ~MCG_C1_CLKS_MASK
			& ~MCG_C1_IREFS_MASK)
			| MCG_C1_CLKS(0)
			| (0 << MCG_C1_IREFS_SHIFT);
		MCG_C6 =(MCG_C6
				& ~MCG_C6_PLLS_MASK)
				| (1 << MCG_C6_PLLS_SHIFT);
		
		
		// Loop until S[PLLST] is set (make sure pll is clock)
		// Loop until S[CLKST] are 2'b11, indicating that the PLL output is selected to feed MCGOUTCLK in the current clock mode.
		while ((MCG_S & MCG_S_PLLST_MASK) != (1 << MCG_S_PLLST_SHIFT));
		while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(3));
		break;
		
	case PBE:
		//Return clock to default value
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV4(1);
		
		// C1[CLKS] bits are written to 10
		// C1[IREFS] bit is written to 0
		// C6[PLLS] bit is written to 1
		// C2[LP] bit is written to 0
		MCG_C1 = (MCG_C1 
			& ~MCG_C1_CLKS_MASK
			& ~MCG_C1_IREFS_MASK)
			| MCG_C1_CLKS(2)
			| (0 << MCG_C1_IREFS_SHIFT);
		MCG_C6 =(MCG_C6
				& ~MCG_C6_PLLS_MASK)
				| (1 << MCG_C6_PLLS_SHIFT);
		MCG_C2 = (MCG_C2
				& ~MCG_C2_LP_MASK)
				| (0 << MCG_C2_LP_SHIFT);
		
		// Loop until S[PLLST] is set, indicating that the current source for the PLLS clock is the PLL.
		// Loop until S[CLKST] is 2'b10, indicating that the external reference clock is selected to feed MCGOUTCLK.
		while ((MCG_S & MCG_S_PLLST_MASK) != (1 << MCG_S_PLLST_SHIFT));
		while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(2));
		break;
		
	case BLPI:
		// C1[CLKS] bits are written to 01
		// C1[IREFS] bit is written to 1
		// C6[PLLS] bit is written to 0
		// C2[LP] bit is written to 1
		// The top 3 conditions should be met by FBI already
		MCG_C2 = (MCG_C2
				& ~MCG_C2_LP_MASK)
				| (1 << MCG_C2_LP_SHIFT);
		break;
		
	case BLPE:
		// C1[CLKS] bits are written to 10
		// C1[IREFS] bit is written to 0
		// C2[LP] bit is written to 1
		MCG_C1 = (MCG_C1 
			& ~MCG_C1_CLKS_MASK
			& ~MCG_C1_IREFS_MASK)
			| MCG_C1_CLKS(2)
			| (0 << MCG_C1_IREFS_SHIFT);
		MCG_C2 = (MCG_C2
				& ~MCG_C2_LP_MASK)
				| (1 << MCG_C2_LP_SHIFT);
		
		//Loop until S[IREFST] is 0, indicating the external reference is the current source for the reference clock.
		while ((MCG_S & MCG_S_IREFST_MASK) != (0 << MCG_S_IREFST_SHIFT)); 
		break;
		
	default:
		//should never occur
		break;
	}
}







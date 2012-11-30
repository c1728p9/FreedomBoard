/*
 * main implementation: use this 'C' sample to create your own application
 *
 */



/*#include <stdio.h>*/ /* uncomment to enable 'puts' below */

#include "derivative.h" /* include peripheral declarations */

void busy_sleep(int time);

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


void init_uart();


static clk_mode_t get_clock_state();

static void init_leds() {
	//Enable port configuration clock
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;
	
	//Configure port c19 (Green) as an gpio, output, set (off)
	PORTB_PCR19 = (1 << PORT_PCR_MUX_SHIFT) | (PORTB_PCR19 & ~PORT_PCR_MUX_MASK);
	GPIOB_PDDR |= (1 << 19);//PTB19
	GPIOB_PDOR |= (1 << 19);
	
	PORTB_PCR18 = (1 << PORT_PCR_MUX_SHIFT) | (PORTB_PCR18 & ~PORT_PCR_MUX_MASK);
	GPIOB_PDDR |= (1 << 18);//PTB18
	GPIOB_PDOR |= (1 << 18);
	
	PORTD_PCR1 = (1 << PORT_PCR_MUX_SHIFT) | (PORTD_PCR1 & ~PORT_PCR_MUX_MASK);
	GPIOD_PDDR |= (1 << 1);//PTD1
	GPIOD_PDOR |= (1 << 1);
	
}
static void set_red(uint8_t on) {
	if (on) {
		GPIOB_PDOR &= ~(1 << 18);
	} else {
		GPIOB_PDOR |= (1 << 18);
	}
}
static void set_green(uint8_t on) {
	if (on) {
		GPIOB_PDOR &= ~(1 << 19);
	} else {
		GPIOB_PDOR |= (1 << 19);
	}
}
static void set_blue(uint8_t on) {
	if (on) {
		GPIOD_PDOR &= ~(1 << 1);
	} else {
		GPIOD_PDOR |= (1 << 1);
	}
}

void blink_lights(int times) {
	while (times > 0) {
		set_green(1);
		busy_sleep(1000000);
		set_green(0);
		busy_sleep(1000000);
		times--;
	}
}
void blink_lights_fast(int times) {
	while (times > 0) {
		set_green(1);
		busy_sleep(1000);
		set_green(0);
		busy_sleep(1000);
		times--;
	}
}
void error() {
	set_red(1);
	while (1);
}

int main(void)
 {
//CHECK TO SEE IF BIT 0 OF A FUNCTION POITER IS 1 FOR ARM and 0 FOR THUMB
	
	
	init_leds();
	init_clock();
	
	set_red(1);
	busy_sleep(200000);
	set_red(0);
	set_green(1);
	busy_sleep(200000);
	set_green(0);
	set_blue(1);
	busy_sleep(200000);
	set_blue(0);
	
	
//	if (get_clock_state() != FEI)
//		error();
//	
//	//FEI
//	blink_lights(3);
	
	//clock_mode(FEI);
	
	//FEI again
	//blink_lights(3);
	
	//FBE
//	clock_mode(FBE);
//	blink_lights(3);
//	
//	if (get_clock_state() != FBE)
//		error();
	
	//FEE
//	clock_mode(FEE);
//	blink_lights(3);
//	
//	if (get_clock_state() != FEE)
//		error();
	
	//FBI
//	clock_mode(FBI);
//	blink_lights_fast(3);
//	if (get_clock_state() != FBI)
//		error();
	
//	clock_mode(BLPI);
//	blink_lights_fast(3);
//	if (get_clock_state() != BLPI)
//		error();
	
//	clock_mode(BLPE);
//	blink_lights(3);
//	if (get_clock_state() != BLPE)
//		error();
	
	//PBE
//	clock_mode(PBE);
//	blink_lights(3);
//	if (get_clock_state() != PBE)
//		error();
	
	//PEE
	clock_mode(PEE);
	blink_lights(3);
	if (get_clock_state() != PEE)
		error();
	
	//MCG
	

	
	init_uart();
	
	while (1) {
		set_blue(1);
		busy_sleep(100000);
		set_blue(0);
		busy_sleep(100000);
	}
	
	
	/* Enabling 'puts' below will use the UART/SCI of your target.
	 * Make sure you correctly set it up or how to use the Debug Console instead.
	 * See the targeting manual for details.
	 */
	/*puts("Hello (Kinetis) World in 'C' from MKL25Z128 derivative!");*/
	

	
	return 0;
}

void init_uart() {
	
#define XTAL_FREQ 8000000 
#define INTERNAL_RC 4000000
#define PLL_FREQ 48000000
	
	//Enable uart clock gate
	SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;
	
	//Enable PORTA (uart tx pin needs to be configured using this port)
	SIM_SCGC5 = (SIM_SCGC5 & SIM_SCGC5_PORTA_MASK) |
					(1 << SIM_SCGC5_PORTA_SHIFT);
	
	
	///TEMP  - turn on fast internal reference clock
	MCG_SC &= ~MCG_SC_FCRDIV_MASK;
	MCG_C2 |= MCG_C2_IRCS_MASK;
	MCG_C1 |= MCG_C1_IRCLKEN_MASK;
	
	
	
	//Select external oscillator as uart clock
	SIM_SOPT2 = (SIM_SOPT2 
			& ~SIM_SOPT2_UART0SRC_MASK
			& ~SIM_SOPT2_PLLFLLSEL_MASK)
			| (1 << SIM_SOPT2_PLLFLLSEL_SHIFT)
			| SIM_SOPT2_UART0SRC(1);//2 for external osc //3 for int //1 for fll/pll
	
	//PTA2 go to ALT2
	PORTA_PCR2 = (PORTA_PCR2 & PORT_PCR_MUX_MASK) |
			PORT_PCR_MUX(2);
	
	//PTA1 go to ALT2
	PORTA_PCR1 = (PORTA_PCR1 & PORT_PCR_MUX_MASK) |
			PORT_PCR_MUX(2);
	
	
	//make sure transmitter and reciever are disabled
	
	//Baud rate = baud clock / ((OSR+1) × BR)
	//Target baud rate =  115200, clock = 48MHz
	//NOTE: transmitter is the baudrate clock divided by oversampling rate
	int TARGET_BAUD = 115200;
	
	int OVERSAMPLING = 16;//SHOULD be 16?
	const int BAUD_CLOCK = PLL_FREQ;
	
	int BR = BAUD_CLOCK / (TARGET_BAUD * OVERSAMPLING);
			
	//make sure value is in range (not implemented yet)
			
	UART0_BDH = UART_BDH_SBR( (BR >> 8) & 0xFF ) ;
	UART0_BDL = (BR >> 0) & 0xFF;
	
	UART0_C4 = OVERSAMPLING - 1;
	
	UART0_C2 |= UART_C2_RIE_MASK;
	
	NVIC_ISER |= (1 << 12);
	
	//Enabled transmitter
	UART0_C2 |= UART_C2_TE_MASK | UART_C2_RE_MASK;
	
	UART0_D = 'a';
	
	//SEND TEST DATA
//	while (1) {
//		uint8_t temp;
//		while (!(UART0_S1 & UART_S1_RDRF_MASK));
//		temp = UART0_D;
//		UART0_D = temp;
//		
//		//UART0_D = 'a';
//		//while (!(UART0_S1 & UART_S1_TDRE_MASK));
//		//UART0_D = ~temp;
//	}
	
}

void UART0_IRQHandler() {
	//TDRE and TC
	//RDRF, IDLE, RXEDGIF, and LBKDIF
	//OR, NF, FE,	and PF
	uint8_t temp;
	temp = UART0_D;
	UART0_D = temp;
}


void busy_sleep(int time) {
	volatile int i = 0;
	for (i = 0; i < time; i++);
}

////////////////////////////////////////////////////////////
//					Clock submodule
////////////////////////////////////////////////////////////

static clk_mode_t get_clock_state();
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
		
	} else {
		error();
	}
}

void clock_mode(clk_mode_t target_state) {
	
//	MCG_C1 = (MCG_C1 & MCG_C1_CLKS_MASK) |
//			MCG_C1_CLKS(2);//Select new clock
	
	//NOTE: flash clock must be less than or equal to 24 MHz
	
	
// C1[CLKS] - use to transition to/from bypass mode.
// select internal, external (bypass mode FBI or FBE) or fll/pll (engaged mode)
// changing this register effectively selects 
// C1[IREFS] selects the source for FLL (FEI or FEE)
	
// C6[PLLS] - used to select FLL or PLL (tansition between FBE and PBE)
	
// C2[LP] - Transitions to low power mode (BLPI and BLPE)
	
//				____FLL____ 	___PLL____		__NEITHER__
//				INT		EXT		INT		EXT		INT		EXT
//			
//Enabled		FEI,	FEE,  	/*PEI*/	PEE,  
//Bypass		FBI,	FBE,	/*PBI*/	PBE,  
//Low power									BLPI,	BLPE, Stop
		
		
		
//	CLKST //fll, int, ext, pll
//
//	FEI, //FLL Engaged Internal - 24MHz (from FLL using 32khz internal)
//	FBE, //FLL Bypassed External - 8MHz external
//	PBE, //PLL Bypassed External
//	PEE //PLL Engaged External
	
	clk_mode_t current_state = get_clock_state();
	while (current_state != target_state) {
		clk_mode_t next_state = get_next_clock_state(current_state, target_state);
		set_clock_state(next_state);
		current_state = get_clock_state();
	}
}

static clk_mode_t get_clock_state() {
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
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1) | SIM_CLKDIV1_OUTDIV4(2);
		
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
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV4(2);
		
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
	//1. First, FEI must transition to FBE mode:
	//Loop until S[OSCINIT0] is 1, indicating the crystal selected by C2[EREFS0] has been initialized.
	//Loop until S[IREFST] is 0, indicating the external reference is the current source for the reference clock.
	//Loop until S[CLKST] is 2'b10, indicating that the external reference clock is selected to feed MCGOUTCLK.
	
	//3. Then, FBE must transition either directly to PBE mode or first through BLPE mode and then to PBE mode:
	//Loop until S[PLLST] is set, indicating that the current source for the PLLS clock is the PLL.
	//loop until S[LOCK0] is set, indicating that the PLL has acquired lock.
	
	
	//4. Lastly, PBE mode transitions into PEE mode:
	//Loop until S[CLKST] are 2'b11, indicating that the PLL output is selected to feed MCGOUTCLK in the current clock mode.
}





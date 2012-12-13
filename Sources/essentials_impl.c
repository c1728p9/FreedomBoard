

#include "derivative.h"
#include "essentials.h"

#define UART0_TX_BUFF_SIZE 4000
uint8_t uart0_tx_buff[UART0_TX_BUFF_SIZE];
circ_buf_t uart0_buff = 
{
    uart0_tx_buff,
    sizeof(uart0_tx_buff),
    0,
    0
};



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
    
    int OVERSAMPLING = 16;
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

}



int send_data(const uint8_t* data, int size) {
    int result = circ_buf_write(&uart0_buff,data,size);
    UART0_C2 |= UART_C2_TIE_MASK;
    return result;
}



void UART0_IRQHandler() {

    if (UART0_S1 & UART_S1_TDRE_MASK ) {

        int byte = circ_buf_read_byte(&uart0_buff);
        if (byte >= 0) {
            UART0_D = byte;
        } else {
            UART0_C2 &= ~UART_C2_TIE_MASK;
        }
        
    }
    if (UART0_S1 & UART_S1_RDRF_MASK){
        UART0_D;
    }
   
    
}



void init_timer() {
    
#define TIMER_CLOCK     24000000
    SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
    PIT_MCR = 0;
    PIT_LDVAL(0) = (TIMER_CLOCK / 1000) - 1; //1ms
    NVIC_ISER |= (1 << (INT_PIT - 16));
    PIT_TCTRL(0) = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
}

void PIT_IRQHandler() {
    if (PIT_TFLG(0) & PIT_TFLG_TIF_MASK) {
        
        //Clear the interrupt
        PIT_TFLG(0)|= PIT_TFLG_TIF_MASK;

        //Run the event scheduler
        run_scheduler();
        
    } else if (PIT_TFLG(1) & PIT_TFLG_TIF_MASK) {
        PIT_TFLG(1) |= PIT_TFLG_TIF_MASK;
        
    }
}

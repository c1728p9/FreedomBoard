/*
 * main implementation: use this 'C' sample to create your own application
 *
 */



/*#include <stdio.h>*/ /* uncomment to enable 'puts' below */

#include "derivative.h" /* include peripheral declarations */

#include "essentials.h"
#include "clock.h"

static void init_leds();
static void busy_sleep(int time);
static void set_red(uint8_t on);
static void set_green(uint8_t on);
static void set_blue(uint8_t on);
void blink_lights(int times);


void periodic(void* data) {
    static int toggle = 0;
    printf("periodic task called at time = %i\n", (int) get_time_ms());
    printf("queue usage = %i\n", tq_max_size());
    set_blue(toggle);
    toggle = !toggle;
}

int main(void)
 {	
	init_leds();
	init_clock(); //Everything needs a clock
	
	set_red(1);	busy_sleep(100000);	set_red(0);
	set_green(1); busy_sleep(100000); set_green(0);
	set_blue(1); busy_sleep(100000); set_blue(0);
	
    init_uart();        //Initalize the printf uart second (this is used to report errors in other modules)
    init_essentials();  
	init_timer();
	
    
    timer_task_t task;
    create_periodic_task(&task,0,500,periodic,0);
	
	printf("does printf work? i=%i\n",28);
	
	
	while (1) {
	    tq_run(0);
	}
	
	return 0;
}


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

void busy_sleep(int time) {
	volatile int i = 0;
	for (i = 0; i < time; i++);
}







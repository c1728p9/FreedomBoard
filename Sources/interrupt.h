/*
 * interrupt.h
 *
 *  Created on: Dec 1, 2012
 *      Author: Russ
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#define disable_interrupt()		__asm__("CPSID i")
#define enable_interrupt() 		__asm__("CPSIE i")
#define interrupt_enabled()     (!interrupt_disabled())


extern uint32_t interrupt_disabled(void);




#endif /* INTERRUPT_H_ */

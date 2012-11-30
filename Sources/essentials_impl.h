#ifndef ESSENTIALS_IMPL_H_
#define ESSENTIALS_IMPL_H_

// This file is included by essentials.h and is meant to 
// macro or provide a prototype for the functions 
// the essential module needs to implement.

#define TASK_QUEUE_SIZE     10

#include "interrupt.h"

int send_data(const uint8_t* data, int size);

//User functions
void init_uart();
void init_timer();


#endif /* ESSENTIALS_IMPL_H_ */

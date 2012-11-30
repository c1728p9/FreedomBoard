

#ifndef TIMING_H_
#define TIMING_H_

#include "queue.h"

typedef struct timer_task_struct {
	//struct timer_task_struct* next;
	//struct timer_task_struct* prev;
	//int32_t time;
	//int32_t count;
	pq_entry_t queue_entry;
	void (*function) (void*);
	void* data;
	uint32_t period;
	uint8_t active;
	uint8_t type;
} timer_task_t;

void init_scheduler(void);

//call this every ms to run the scheduler.
//This must be called from the main queue
void run_scheduler(void);

//initializes an empty task
//returns 0 on succes (always returns 0)
int create_empty_task(timer_task_t* new_task);

//creates a new periodic task at the start time and with the given period
//return 0 on success
int create_periodic_task(timer_task_t* new_task,uint64_t start_time_ms,uint32_t period, void (*function)(void*), void* data);


//creates a new wdt task.
//if the task is not cleared within the period it calls the given function
//returns 0 on success
int create_wtd_task(timer_task_t* new_task, uint32_t timeout_time, void (*function)(void*), void* data);


//resets the wdt task as to prevent it from running its function
//the task will be called at timeout_time from this function is it is not called again
//returns 0 if the task was cleared
//returns 1 if the task is running
//returns 2 if the task had completed
//returns a negative number on failure 
int clear_wtd_task(timer_task_t* new_task);

//remove the given periodic task. 
//this function is safe to call from within a periodic task
//return 0 on success, return 1 the task is currently being execute (if you called remove_task within its own timer task) and
//return 2 if the task has already been run and is finished
int remove_task(timer_task_t* new_task);


//checks if the given timer task is active or inactive
//return non-zero value if the task is active
//zero if hte task is inactive
int task_is_active(timer_task_t* new_task);

//create a one-shot task
//note-tasks are called in an interrrupt with other interrupts disabled, so tasks should be brief.
//return 0 on success
int create_timer_task(timer_task_t* new_task,uint64_t start_time_ms, void (*function)(void*), void* data);
//int remove_timer_task() //implement when needed
//int timer_task_finished() //implement when needed

//returns the number of milliseconds since the timer was started
uint64_t get_time_ms(void);

#endif /* TIMING_H_ */

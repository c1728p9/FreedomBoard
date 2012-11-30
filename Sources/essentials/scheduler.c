
#include "utilities.h"
#include "queue.h"

//interface
#include "scheduler.h"


enum {
	TASK_TYPE_SINGLE = 0,
	TASK_TYPE_PERIODIC = 1,
	TASK_TYPE_WDT = 2,
	TASK_TYPE_FINISHED = 3
};


static uint64_t ms_since_start = 0;
timer_task_t* current_running_task = 0;

//priority queue
//lower number = higher priority
//priority = seconds since start
pq_queue_t timing_queue;

void init_scheduler() {
	int result = 0;
	
	//create timer queue
	result = pq_create(&timing_queue);
	if (result != 0) warning("possible error creating timer queue");

}


int create_empty_task(timer_task_t* new_task) {
	//AVR32_ENTER_CRITICAL_REGION();
	new_task->function = 0;
	new_task->data = 0;
	new_task->type = TASK_TYPE_FINISHED;
	new_task->period = 0;
	new_task->active = false;
	//AVR32_LEAVE_CRITICAL_REGION();
	return 0;
}

int create_periodic_task(timer_task_t* new_task,uint64_t start_time_ms,uint32_t period, void (*function)(void*), void* data) {
	int result;
	
	new_task->function = function;
	new_task->data = data;
	new_task->type = TASK_TYPE_PERIODIC;
	new_task->period = period;
	result = pq_add_back(&timing_queue,(pq_entry_t*)new_task,start_time_ms);
	if (result == 0)
		new_task->active = true;
	else
		warning("Error with adding task to queue");
		
	return result;
}

int create_timer_task(timer_task_t* new_task,uint64_t start_time_ms, void (*function)(void*), void* data) {
	int result;
	
	new_task->function = function;
	new_task->data = data;
	new_task->type = TASK_TYPE_SINGLE;
	new_task->period = 0;
	result = pq_add_back(&timing_queue,(pq_entry_t*)new_task,start_time_ms);
	if (result == 0)
		new_task->active = true;
	else
		warning("Error with adding task to queue");
		
	return result;
}

int create_wtd_task(timer_task_t* new_task, uint32_t timeout_time, void (*function)(void*), void* data) {
	int result;
	
	new_task->function = function;
	new_task->data = data;
	new_task->type = TASK_TYPE_WDT;
	new_task->period = timeout_time;
	result = pq_add_back(&timing_queue,&(new_task->queue_entry),get_time_ms() + timeout_time);
	if (result == 0)
		new_task->active = true;
	else
		warning("Error with adding task to queue");
		
	return result;
}

int clear_wtd_task(timer_task_t* task) {
	int result;
	
	result = remove_task(task);
	if (result == 0) {
		//if the task has not been run then re-add
		int result = pq_add_back(&timing_queue, &(task->queue_entry), get_time_ms() + task->period);
		if (result == 0)
			task->active = true;
		else 
			result = -1;
	}

	return result;
}

int remove_task(timer_task_t* task) {
	int result = 0;
	
	result = pq_remove_element(&(task->queue_entry));
	if (result < 0) goto exit;
	else if (current_running_task == task) result = 1;	//task was removed while running - return 1
	else if (result == 1) result = 2;					//task has already run - return 2
		
	//set to inactive state
	task->type = TASK_TYPE_FINISHED;
	task->active = false;
	
	exit:
	return result;
}

int task_is_active(timer_task_t* task) {
	return task->active;
}


void run_scheduler_impl(void* dumby);
void run_scheduler(void) {
    tq_post(run_scheduler_impl, 0);
}

//this runs any tasks in the one-time task queue that are ready to be run
void run_scheduler_impl(void* dumby) {
	
	//keep track of time
	ms_since_start++;
	
	//run all valid ready tasks
	timer_task_t* p_task = (timer_task_t*) pq_peek(&timing_queue);
	while ((p_task != 0) && (pq_get_priority(&(p_task->queue_entry)) <= ms_since_start)) {
		
		//sanity check - make sure task is running at correct time
		if (pq_get_priority(&(p_task->queue_entry)) != ms_since_start)
			warning("Task getting run at incorrect time");
	
		//run task if valid
		current_running_task = p_task;
		if (p_task->function != 0) p_task->function(p_task->data);
		else warning("Null function in timed task");
		current_running_task = 0;
		
		//if task has not been removed
		if (pq_is_in_queue((pq_queue_t*) &timing_queue,(pq_entry_t*) p_task) != 0) {
			
			//remove task from queue
			if (&(p_task->queue_entry) == pq_peek((pq_queue_t*) &timing_queue)) {
				pq_remove_front(&timing_queue);
			} else {
				pq_remove_element((pq_entry_t*)&p_task);
			}	
			
			//re-add periodic tasks, or mark finished if no periodic
			if (p_task->type == TASK_TYPE_PERIODIC) {
				int result;
				result = pq_add_back(&timing_queue,(pq_entry_t*) p_task,pq_get_priority(&(p_task->queue_entry)) + p_task->period);
				if (result != 0) warning("Error re-adding periodic task");
			} else {
				p_task->active = false;
			}
		}
		
		//get next task
		p_task = (timer_task_t*) pq_peek(&timing_queue);
	}
	
}


uint64_t get_time_ms() {
	return ms_since_start;
}











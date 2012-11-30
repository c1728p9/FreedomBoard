
#ifndef QUEUE_H_
#define QUEUE_H_

#include<stdint.h>

#include "utilities.h"

typedef struct queue_entry_struct {
	struct queue_entry_struct* next;
	void* data;
} queue_entry_t;

struct _queue_t {
	queue_entry_t* first;
	queue_entry_t* last;
	int size;
};
struct _queue_t;
typedef struct _queue_t queue_t;


//initialize the queue.  Return zero on success
//currently only returns 0
int create_queue(queue_t* queue);

//void delete_queue(queue_t* queue);

//adds an element tot the back of the queue.
//returns 0 if successful, positive number for warning,
//negative number for failure.
int add_element(queue_t* queue, queue_entry_t* element);

//gets an element from the queue or returns 0 if the queue is empty
queue_entry_t* get_element(queue_t* queue);
int get_size(queue_t* queue);



//////////////////////////////////////////////////////////////////////////
//						Priority Queue Code								//
//////////////////////////////////////////////////////////////////////////


struct pq_queue_struct;
typedef struct pq_entry_struct {
	struct pq_entry_struct* next;
	struct pq_entry_struct* prev;
	struct pq_queue_struct* queue;
	uint64_t priority;
} pq_entry_t;


typedef struct pq_queue_struct {
	int size;
	pq_entry_t* front;
	pq_entry_t* back;
} pq_queue_t;


//return 0 on success unless otherwise specified
//lower value = higher priority - list will be in order from least to greatest

int pq_create(pq_queue_t* queue);
int pq_destroy(pq_queue_t* queue);

inline
int pq_size(pq_queue_t* queue);


// return front element or null of none
inline
pq_entry_t* pq_peek(pq_queue_t* queue);

int pq_add_front(pq_queue_t* queue,pq_entry_t* element, uint64_t priority);
int pq_add_back(pq_queue_t* queue,pq_entry_t* element, uint64_t priority);

pq_entry_t* pq_remove_back(pq_queue_t* queue);
pq_entry_t* pq_remove_front(pq_queue_t* queue);

//removes the element from its queueu
//returns 1 if the element was not in the queue
//returns 0 if the element was successfully removed
//returns a negative value on failure
int pq_remove_element(pq_entry_t* element);


#define pq_size(queue)                  ((queue)->size)
#define pq_peek(queue)                  ((queue)->front)
#define pq_get_priority(queue)          ((queue)->priority)

//add is in queue
//checks if the given element is in the queue
//return 1 if the element is in the queue, else returns 0

bool pq_is_in_queue(pq_queue_t* queue, pq_entry_t* element);







#endif /* QUEUE_H_ */

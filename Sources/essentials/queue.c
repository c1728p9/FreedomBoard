#include <stdlib.h>

#include "utilities.h"

// Interface
#include "queue.h"

#define QUEUE_DEBUG

int create_queue(queue_t* queue) {
	queue->first = 0;
	queue->last = 0;
	queue->size = 0;
	return 0;
}

//void delete_queue(queue_t* queue);


int add_element(queue_t* queue, queue_entry_t* entry) {
	
	entry->next = 0;
	if (queue->first == 0) {
		//list is empty
		queue->last = queue->first = entry;
	} else {
		//there is at least 1 element
		queue->last->next = entry;		//add element
		queue->last = entry;			//update last pointer
	}
	queue->size++;
	return 0;
}

queue_entry_t* get_element(queue_t* queue) {
	if (queue->first == 0) return 0;			//list is empty
	
	
	queue_entry_t* element = queue->first;	//get element
	queue->first = element->next;			//update first pointer
	if (queue->first == 0) queue->last = 0;	//update last pointer if queue empty
	
	queue->size--;
	return element;
}

int get_size(queue_t* queue) {
	return queue->size;
}



//////////////////////////////////////////////////////////////////////////
//						Priority Queue Code								//
//////////////////////////////////////////////////////////////////////////




int pq_create(pq_queue_t* queue) {
	queue->front = 0;
	queue->back = 0;
	queue->size = 0;
	return 0;
}

int pq_add_front(pq_queue_t* queue,pq_entry_t* new_element, uint64_t priority) {
	
	if (new_element->queue == queue)
		printf("Trying to add duplicate element = %p\r\n",new_element);
	
	new_element->next = 0;
	new_element->prev = 0;
	new_element->priority = priority;
	new_element->queue = queue;

	if (queue->front == 0) {
		//queue is empty - add it as only element
		queue->back = queue->front = new_element;
	} else {
		//queue has at least 1 element - 3 cases
			//element will become new first element (higher priority than first element)
			//elememnt will become new last element (current will be null after iteration)
			//element will be inserted in the middle (higher priority than given element


		
		pq_entry_t* current = queue->front;
		if (priority <= current->priority) {
			//this element should become the new first element
			current->prev = new_element;
			queue->front = new_element;
			new_element->next = current;
		} else {
			//this element will be inserted either at the middle or end of the list
			while ((current != 0) && (priority > current->priority))//find pos where current element has greater priority number then new element
				current = current->next;

			if (current == 0) {
				//insert at back of list
				queue->back->next = new_element;
				new_element->prev = queue->back;
				queue->back = new_element;
			} else {
				//insert in middle of list (before current)
				current->prev->next = new_element;
				new_element->prev = current->prev;
				current->prev = new_element;
				new_element->next = current;
			}

		}
		
	}
	
	//////////////////////////////////////////////////////////////////////////
	//check queue code - make sure queue is intact
	#ifdef QUEUE_DEBUG
	
	pq_entry_t* current = queue->front;
	int count = 0;
	while ((count < 100) && (current != 0) ) {
		count++;
		current = current->next;
	}
	if (count > 90)
		warning("Element broke queue");
		
	#endif
	//////////////////////////////////////////////////////////////////////////
	
	queue->size++;
	return 0;
}



int pq_add_back(pq_queue_t* queue,pq_entry_t* new_element, uint64_t priority) {
	return pq_add_front(queue,new_element, priority);
}

//made inline
/*
int pq_size(pq_queue_t* queue) {
	return queue->size;
}
*/

pq_entry_t* pq_remove_front(pq_queue_t* queue) {
	pq_entry_t* queue_entry;
	if (queue->front == 0) return 0; //no entries

	queue_entry = queue->front;

	//update front pointer
	queue->front = queue_entry->next;

	//if last element update back pointer
	//otherwise update next element
	if (queue_entry->next == 0) {
		queue->back = 0;
	} else {
		queue_entry->next->prev = 0;
	}

	queue->size--;
	queue_entry->queue = 0;
	return queue_entry;
}

int pq_remove_element(pq_entry_t* element) {
	if (element->queue == 0) return 1;

	if (element->next == 0) {
		//this was last element - set last pointer
		element->queue->back = element->prev;
	} else {
		//there is an element after this - make it point to the previous element
		element->next->prev = element->prev;
	}

	if (element->prev == 0) {
		//this was the first element - set the front pointer
		element->queue->front = element->next;
	} else {
		//there is an element before this - make it point to the next element
		element->prev->next = element->next;
	}
	element->queue->size--;
	element->queue = 0;
	element->next = 0;//for test
	element->prev = 0;//for test
	return 0;
}


int pq_destroy(pq_queue_t* queue) {
	return 0;
}
//changed to inline
/*
pq_entry_t* pq_peek(pq_queue_t* queue) {
	return queue->front;
}

uint64_t pq_get_priority(pq_entry_t* element) {
	return element->priority;
}
*/

bool pq_is_in_queue(pq_queue_t* queue, pq_entry_t* element) {
    return ((element->queue == queue)?1:0);
}

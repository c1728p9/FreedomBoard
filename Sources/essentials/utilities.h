
#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <stdlib.h>

#include "../essentials_impl.h"

//TODO temp
typedef uint8_t bool;
#define false           0
#define true            1


#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)










void print_debug_stats(int period);


//////////////////////////////////////////////////////////////////////////
//                      Sanity checking tools                           //
//////////////////////////////////////////////////////////////////////////


//private function
void error_and_exit(char* message, int size);

#define warning(str) send_data((uint8_t*) str "\r\n",sizeof(str "\r\n"));
#define error(str) 
//(send_data("Fatal Error: " str "\r\nError occured in " __FILE__ ", at line "  S__LINE__ "\r\n", 
//sizeof("Fatal Error: " str "\r\nError occured in file " __FILE__ ", at line "  S__LINE__ "\r\n") - 1), while(1) { }




//////////////////////////////////////////////////////////////////////////
//                      Atomic operation utilities                      //
//////////////////////////////////////////////////////////////////////////



#define ENTER_CRITICAL_REGION()                                          \
    {                                                                    \
        int global_interrupt_enabled = interrupt_enabled();              \
        disable_interrupt();


#define LEAVE_CRITICAL_REGION()                                          \
        if (global_interrupt_enabled) enable_interrupt();                \
    }




//////////////////////////////////////////////////////////////////////////
//                      Standard library tweaks                         //
//////////////////////////////////////////////////////////////////////////

// Malloc debugging tool

extern int debug_malloc_count;

static inline 
void* debug_malloc(int size,const char* msg) {
    void* result = malloc(size);
    if (result != 0) debug_malloc_count++;
    return result;
}
static inline
void debug_free(void* data,char* msg) {
    if (data != 0) debug_malloc_count--;
    free(data);
}

#define malloc(val) debug_malloc(val,"Malloc called in " __FILE__ ", at line "  S__LINE__ ":")
#define free(val) debug_free(val,"Free called in " __FILE__ ", at line "  S__LINE__ ":")


// Faster, non blocking version of printf

#define printf mprintf
void mprintf(const char* format, ...);


//////////////////////////////////////////////////////////////////////////
//                      Circular Buffer Code                            //
//////////////////////////////////////////////////////////////////////////

// The contents of this buffer must be set explicitly
// (currently there is no function to do it)
typedef struct {
    uint8_t* buffer;
    int size;           //size of buffer
    int head;           //head position (should be 0 initially)
    int tail;           //tail position (should be 0 initially)
} circ_buf_t;


int buf_get_used(int head, int tail, int size);
int buf_get_free(int head, int tail, int size);



int circ_buf_get_used(circ_buf_t* circ_buf);
int circ_buf_get_free(circ_buf_t* circ_buf);


// These circ_buf_read and circ_buf_writeare used for circular 
// buffer management.
// If integer access is atomic then these functions are safe 
// to use in read-write pairs with one in an interrupt and one 
// in non-interrupt code.

// Reads at most count bytes into data.  Returns the number of bytes read
// or a negative number on failure
int circ_buf_read(circ_buf_t* circ_buf, uint8_t* data, int count);

// Write count bytes from data into the circular buffer
// Return 0 on success or -1 on failure
int circ_buf_write(circ_buf_t* circ_buf,const uint8_t* data, int count);

// Read a byte from the buffer.
// Return byte if successful (positive) or -1 if buffer is empty.
int circ_buf_read_byte(circ_buf_t* circ_buf);

// Write a byte to the buffer.
// Return 0 if successful, -1 otherwise
int circ_buf_write_byte(circ_buf_t* circ_buf, uint8_t byte);


//////////////////////////////////////////////////////////////////////////
//                      Task Queue Code                                 //
//////////////////////////////////////////////////////////////////////////

// Run a task from the task queue.
// This is meant to be called from the main
// thread in a constant loop.fuction
// If a non zero value is passed in this 
// function will put the processor in sleep mode
// if the queue is empty.
// Note: This function must not be called outside the main loop
// or the queues contents could be corrupted.
void tq_run(int sleep);

// Posts the given function pointer and data on the queue.
// Returns 0 on success, returns -1 if there is no space 
// left in the queue.
// Note: This function is synchronized so it is 
// safe to call in any context
int tq_post(void (*funct)(void* ), void* data);

// Returns the maximum usage of the task queue
int tq_max_size(void);



//////////////////////////////////////////////////////////////////////////
//                      Misc functions                                  //
//////////////////////////////////////////////////////////////////////////

// Initializes all elements of the essentials package
// returns 0 on success
int init_essentials(void);

#endif /* UTILITIES_H_ */

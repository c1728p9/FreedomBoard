

#include <stdio.h>
#include <string.h>

#include "scheduler.h"
#include "utilities.h"

int debug_malloc_count = 0;

void print_stats_task(void* data);
timer_task_t print_stats_timer;
void print_debug_stats(int period) {
    int result;
    result = create_periodic_task(&print_stats_timer,get_time_ms() + 500, period, print_stats_task,0);
    if (result != 0) warning("Error creating debug periodic task");
}


//This function is meant for circular buffers.  Returns the used size (bytes written)
int buf_get_used(int head, int tail, int size) {
    if (head >= size) head -= size;
    int used = head - tail;
    if (used < 0) used += size;
    return used;
}

//This function is meant for circular buffers.  Returns the size left minus 1 (so head position won't get confused)
int buf_get_free(int head, int tail, int size) {
    if (head >= size) head -= size;
    int used = tail - head - 1;
    if (used < 0) used += size;
    return used;
}


void mprintf(const char* format, ...) {
    
    #define MAX_PRINTF_LEN 100
    char buf[MAX_PRINTF_LEN];
    
    //generate message
    va_list args;
    va_start (args, format);
    int msg_size = vsnprintf (buf,MAX_PRINTF_LEN,format, args);
    va_end (args);
    
    //send message
    send_data((uint8_t*) buf,msg_size);
}



//////////////////////////////////////////////////////////////////////////
//                      Circular Buffer Code                            //
//////////////////////////////////////////////////////////////////////////


int circ_buf_get_used(circ_buf_t* circ_buf) {
    return buf_get_used(circ_buf->head, circ_buf->tail, circ_buf->size);
}
int circ_buf_get_free(circ_buf_t* circ_buf) {
    return buf_get_free(circ_buf->head,circ_buf->tail,circ_buf->size);
}

int circ_buf_read(circ_buf_t* circ_buf, uint8_t* data, int count) {

    int head, tail, size;
    //Each integer read must be atomic
    head = circ_buf->head;
    tail = circ_buf->tail;
    size = circ_buf->size;
    
    int used_size = buf_get_used(head, tail, size) ;
    int size_to_read = (used_size > count ? count : used_size);
    int return_size = size_to_read;
    int size_until_buffer_end = size - tail;
    if (size_until_buffer_end >= size_to_read) {
        //copy everything with one memcpy
        memcpy(data,&(circ_buf->buffer[tail]),size_to_read);
        tail += size_to_read;
        if (tail >= size)
            tail -= size;
    } else {
        //use 2 memcpy calls
        memcpy(&data[0], &(circ_buf->buffer[tail]),size_until_buffer_end);
        size_to_read -= size_until_buffer_end;
        memcpy(&data[size_until_buffer_end], &(circ_buf->buffer[0]),size_to_read);
        tail = size_to_read;

    }
    if (tail >= size)
        tail -= size;
    
    //Interger write must be atomic
    circ_buf->tail = tail;
    
    return return_size;

}


int circ_buf_write(circ_buf_t* circ_buf,const uint8_t* data, int count) {

    int head, tail, size;
    //Each integer read must be atomic
    head = circ_buf->head;
    tail = circ_buf->tail;
    size = circ_buf->size;
    
    int free_size = buf_get_free(head, tail,size);
    if (free_size >= count) {
        int size_to_write = count;
        int size_until_buffer_end = size - head;
        if (size_to_write <= size_until_buffer_end) {
            memcpy(&(circ_buf->buffer[head]),&data[0],size_to_write);
            head += size_to_write;
        } else {
            memcpy(&(circ_buf->buffer[head]),&data[0],size_until_buffer_end);
            size_to_write -= size_until_buffer_end;
            memcpy(&(circ_buf->buffer[0]),&data[size_until_buffer_end],size_to_write);
            head = size_to_write;
        }
        if (head >= size)
            head -= size;
        
        //Integer write must be atomic
        circ_buf->head = head;
        
        return 0;
    } else {
        return -1;
    }
} 

int circ_buf_read_byte(circ_buf_t* circ_buf) {
    int head, tail, size, ret;
    //Each integer read must be atomic
    head = circ_buf->head;
    tail = circ_buf->tail;
    size = circ_buf->size;
    
    if (head != tail) {
        //There is at least 1 byte in the buffer
        ret = (int) circ_buf->buffer[tail];
        tail++;
        if (tail >= size)
            tail -= size;
    } else {
        ret = -1;
    }
    
    //Integer write must be atomic
    circ_buf->tail = tail;
    
    return ret;
}

int circ_buf_write_byte(circ_buf_t* circ_buf, uint8_t byte) {
    int head, tail, size, ret, new_head;
    
    //Each integer read must be atomic
    head = circ_buf->head;
    tail = circ_buf->tail;
    size = circ_buf->size;
    
    new_head = head + 1;
    if (new_head >= size)
        new_head -= size;
    
    if (new_head != tail) {
        //There is at least 1 byte free
        circ_buf->buffer[head] = byte;
        ret = 0;
    } else {
        ret = -1;
    }
    
    //Integer write must be atomic
    circ_buf->head = new_head;
    
    return ret;
}



//////////////////////////////////////////////////////////////////////////
//                      Task Queue Code                                 //
//////////////////////////////////////////////////////////////////////////

typedef struct {
    void (*funct)(void* );
    void* data;
} task_queue_entry_t;

static task_queue_entry_t task_queue_buf[TASK_QUEUE_SIZE];
static int max_task_queue_size = 0;
static int tq_head = 0, tq_tail = 0;

void tq_run(int sleep) {
    void (*funct)(void* );
    void* data;
    int tail;
    
    tail = tq_tail;
    if (sleep) {
        //Sleep code goes here
    } else {
        if (tq_head == tail) return;
    }
    
    funct = task_queue_buf[tail].funct;
    data = task_queue_buf[tail].data;
    funct(data);
    tail++;
    if (tail >= TASK_QUEUE_SIZE)
        tail -= TASK_QUEUE_SIZE;
    tq_tail = tail;
    
    
}

// Posts the given function pointer and data on the queue.
// Returns 0 on success, returns -1 if there is no space 
// left in the queue.
int tq_post(void (*funct)(void* ), void* data) {
    int usage;
    int ret_val = 0;
    int next_head;
    
    ENTER_CRITICAL_REGION();
    
    next_head = tq_head + 1;
    if (next_head >= TASK_QUEUE_SIZE)
        next_head -= TASK_QUEUE_SIZE;
    
    if (next_head != tq_tail) {
        task_queue_buf[tq_head].funct = funct;
        task_queue_buf[tq_head].data = data;
    } else {
        ret_val = -1;
    }
    tq_head = next_head;
    
    //record max usage
    usage = buf_get_used(tq_head, tq_tail, TASK_QUEUE_SIZE);
    if (usage > max_task_queue_size) max_task_queue_size = usage;
    
    LEAVE_CRITICAL_REGION();
    
    
    return ret_val;
}

int tq_max_size(void) {
    return max_task_queue_size;
}

//////////////////////////////////////////////////////////////////////////
//                      Misc functions                                  //
//////////////////////////////////////////////////////////////////////////


int init_essentials(void) {
    init_scheduler();
    return 0;
}



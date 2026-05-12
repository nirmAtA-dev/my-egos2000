/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: cooperative multithreading and synchronization
 */

#include <sys/queue.h>
#include "print.c"
#include "thread.h"
/* Student's code goes here (Cooperative Threads). */
/* Define the TCB and helper functions (if needed) for multi-threading. */


void thread_init() {
    /* Student's code goes here (Cooperative Threads). */
    //initialize it to empty state
    TAILQ_INIT(&TCB);
    cv_init(&condition);
    //creating a main thread and inserting to the TAILQ
    struct thread *main_thread = malloc(sizeof(struct thread));
    main_thread->id = 0;
    main_thread->status = THREAD_RUNNING;
    main_thread->entry = NULL;
    main_thread->arg = NULL;
    //we add element at the TAIL
    TAILQ_INSERT_TAIL(&TCB, main_thread, thread_ptr);
    /* Student's code ends here. */
}

void ctx_entry() {
    /* Student's code goes here (Cooperative Threads). */
    struct thread *child_thread = TAILQ_LAST(&TCB, thread_queue);
    if((child_thread->id > 0) && (child_thread->status == THREAD_READY || child_thread->status == THREAD_WAITING)){
        child_thread->status = THREAD_RUNNING;
        child_thread->entry(child_thread->arg);
        thread_exit();
    }

    thread_exit(); 
    /* Student's code ends here. */
}

void thread_create(void (*entry)(void *arg), void *arg) {
    /* Student's code goes here (Cooperative Threads). */
    //first fetch the tail pointed struct 
    struct thread *tail_thread = TAILQ_LAST(&TCB, thread_queue);
    if(tail_thread == NULL){
	    printf("First call thread_init() then thread_create");
       	    _end();
    }
    //create a new child thread
    struct thread *child_thread = malloc(sizeof(struct thread));
    child_thread->id = tail_thread->id + 1;
    child_thread->status = THREAD_READY;
    tail_thread->status = THREAD_WAITING;
    child_thread->entry = entry;
    child_thread->arg = arg;
    TAILQ_INSERT_TAIL(&TCB, child_thread, thread_ptr);
    //we create a child stack
    char* child_stack = malloc(STACK_SIZE);
    ctx_start(&(tail_thread->sp), child_stack + STACK_SIZE);
    /* Student's code ends here. */
}

void thread_yield() {
    /* Student's code goes here (Cooperative Threads). */
    struct thread *current_thread = TAILQ_LAST(&TCB, thread_queue);
    struct thread *prev_thread = TAILQ_PREV(current_thread, thread_queue, thread_ptr); 
    if(current_thread->status == THREAD_RUNNING){
	    current_thread->status = THREAD_WAITING;
	    prev_thread->status = THREAD_RUNNING;
	    ctx_switch(&(current_thread->sp) , prev_thread->sp);
    }else{
	    prev_thread->status = THREAD_WAITING;
	    current_thread->status = THREAD_RUNNING;
	    ctx_switch(&(prev_thread->sp) , current_thread->sp);
    }    

    /* Student's code ends here. */
}

void thread_exit() {
    /* Student's code goes here (Cooperative Threads). */
    struct thread *current_thread = TAILQ_LAST(&TCB, thread_queue);
    struct thread *prev_thread = TAILQ_PREV(current_thread, thread_queue, thread_ptr); 
    if(current_thread->id == 0){
        _end();
    }
    void* switch_sp = prev_thread->sp;
    TAILQ_REMOVE(&TCB, current_thread, thread_ptr);
    free(current_thread);
    void *dummy_ptr;
    ctx_switch(&dummy_ptr, switch_sp);
    /* Student's code ends here. */
}

/* Student's code goes here (Cooperative Threads). */
/* Define helper functions (if needed) for conditional variables. */

/* Student's code ends here. */

void cv_init(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */
    TAILQ_INIT(&(condition->CONDQ));
    /* Student's code ends here. */
}

void cv_wait(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */
    struct thread *current_running_thread = TAILQ_LAST(&TCB, thread_queue);
    if(current_running_thread != NULL){
	    current_running_thread->status = THREAD_WAITING;
	    TAILQ_REMOVE(&TCB, current_running_thread, thread_ptr);
	    TAILQ_INSERT_TAIL(&(condition->CONDQ), current_running_thread, thread_ptr);
    }
    /* Student's code ends here. */
}

void cv_signal(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */
    struct thread *thread_to_wake = TAILQ_LAST(&(condition->CONDQ),thread_queue);
    if(thread_to_wake != NULL){
	    thread_to_wake->status = THREAD_RUNNING;
	    TAILQ_REMOVE(&(condition->CONDQ), thread_to_wake, thread_ptr);
	    TAILQ_INSERT_TAIL(&TCB, thread_to_wake, thread_ptr);
    }

    /* Student's code ends here. */
}

#define BUF_SIZE 3
void* buffer[BUF_SIZE];
int count = 0;
int cvhead = 0, cvtail = 0;
struct cv nonempty, nonfull;

void produce(void* arg) {
    while (1) {
        while (count == BUF_SIZE) cv_wait(&nonfull);
        // At this point, the buffer is not full. 

        // Student's code goes here (Cooperative Threads). 
        // Print out the producer ID with the arg pointer. 

        // Student's code ends here. 
        buffer[cvtail] = arg;
        cvtail = (cvtail + 1) % BUF_SIZE;
        count += 1;
        cv_signal(&nonempty);
    }
}

void consume(void *arg) {
    while (1) {
        while (count == 0) cv_wait(&nonempty);
        // At this point, the buffer is not empty. 

        // Student's code goes here (Cooperative Threads). 
        // Print out the consumer ID with the arg pointer. 

        // Student's code ends here.
        void* result = buffer[cvhead];
        cvhead = (cvhead + 1) % BUF_SIZE;
        count -= 1;
        cv_signal(&nonfull);
    }
}


/*
void child(void* arg) {
    printf("%s is running.\n\r", arg);
}
*/
/*
void child(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("%s is in for loop i=%d\n\r", arg, i);
        thread_yield();
    }
}
*/

int main() {
    // Basic thread functionality
     /*
    thread_init();
    thread_create(child, "Child thread");
    printf("Main thread is running.\n\r");
    thread_exit();
    */
    /*
    thread_init();
    thread_create(child, "Child thread");
    for (int i = 0; i < 10; i++) {
        printf("Main thread is in for loop i=%d\n\r", i);
        thread_yield();
    }
    thread_exit();
    */

    thread_init();

    int ID[500];
    for (int i = 0; i < 500; i++) ID[i] = i;

    for (int i = 0; i < 500; i++)
        thread_create(consume, ID + i);

    for (int i = 0; i < 500; i++)
        thread_create(produce, ID + i);

    printf("main thread exits\n\r");
    thread_exit();

    /* The control flow should NEVER get here. If the main thread is the last to
     * call thread_exit(), thread_exit() should terminate the program by calling
     * the _end() in thread.s.
     * If the main thread is not the last, thread_exit() will switch the context
     * to another thread. Later, when all the threads have called thread_exit(),
     * the last one calling it should then call _end() within thread_exit(). */
}

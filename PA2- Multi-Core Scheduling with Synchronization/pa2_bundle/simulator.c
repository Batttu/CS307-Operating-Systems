#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include "constants.h"
#include "wbq.h"

extern int stop_threads;
extern int finished_jobs[NUM_CORES];
extern int flag_array[NUM_CORES];
extern WorkBalancerQueue** processor_queues;

// Thread function for each core simulator thread
void* processJobs(void* arg) {
    // initalize local variables
    ThreadArguments* my_arg = (ThreadArguments*) arg;
    WorkBalancerQueue* my_queue = my_arg -> q;
    int my_id = my_arg -> id;

    // Main loop, each iteration simulates the processor getting The stop_threads flag
    // is set by the main thread when it concludes all jobs are finished. The bookkeeping 
    // of finished jobs is done in executeJob's example implementation. a task from 
    // its or another processor's queue. After getting a task to execute the thread 
    // should call executeJob to simulate its execution. It is okay if the thread
    //  does busy waiting when its queue is empty and no other job is available
    // outside.
    while (!stop_threads) {
        // TODO: You need to fill in this part
        // .
        // .
        // Use a call to executeJob as stated here.
        
        executeJob(task, my_queue, my_id);            
        
    }
}

// Do any initialization of your shared variables here.
// For example initialization of your queues, any data structures 
// you will use for synchronization etc.
void initSharedVariables() {
    // TODO: Fill in this function according to your needs:
    // .
    // .
}
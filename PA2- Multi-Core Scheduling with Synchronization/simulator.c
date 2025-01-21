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
extern WorkBalancerQueue** processor_queues;

// thread function for each core simulator thread
void* processJobs(void* arg) {
    // initialize local variables
    ThreadArguments* my_arg = (ThreadArguments*) arg;
    WorkBalancerQueue* my_queue = my_arg->q;
    int my_id = my_arg->id;
    free(my_arg);  // free allocated argument

    // load balancing thresholds
    int LOW_WATERMARK = 10;
    int HIGH_WATERMARK = 20;

    while (!stop_threads) {
        // fetch task from own queue
        Task* task = fetchTask(my_queue);

        if (task == NULL) {
            // queue is empty | below low watermark
            // attempt fetch tasks from other cores
            int fetched = 0;

            // check own queue size
            pthread_mutex_lock(&(my_queue->mutex));
            int my_queue_size = my_queue->size;
            pthread_mutex_unlock(&(my_queue->mutex));

            if (my_queue_size < LOW_WATERMARK) {
                for (int i = 0; i < NUM_CORES; i++) {
                    if (i == my_id) continue; // skip own queue

                    // check if other queue is over high watermark
                    WorkBalancerQueue* other_queue = processor_queues[i];

                    // lock other queue to read its size
                    pthread_mutex_lock(&(other_queue->mutex));
                    int other_queue_size = other_queue->size;
                    pthread_mutex_unlock(&(other_queue->mutex));

                    if (other_queue_size > HIGH_WATERMARK) {
                        // try stealing    task
                        Task* stolen_task = fetchTaskFromOthers(other_queue);
                        if (stolen_task != NULL) {
                            task = stolen_task;
                            // reset cache_warmed_up, task is migrated
                            task->cache_warmed_up = 1.0;
                            // update task owner
                            task->owner = my_queue;
                            fetched = 1;
                            break;
                        }
                    }
                }
            }

            if (!fetched) {
                // no tasks to fetch,core can sleep 
                usleep(1000); // sleep for 1 ms
                continue;
            }
        }

        // execute task
        executeJob(task, my_queue, my_id);

        // check if task finished
        if (task->task_duration > 0) {
            // submit task back to own queue
            submitTask(my_queue, task);
        } else {
            // task is finished, free task
            free(task->task_id);
            free(task);
        }
    }

    pthread_exit(NULL);
}

// initialize shared vars and mutexes
void initSharedVariables() {
    for (int i = 0; i < NUM_CORES; i++) {
        pthread_mutex_init(&(processor_queues[i]->mutex), NULL);
        processor_queues[i]->head = NULL;
        processor_queues[i]->tail = NULL;
        processor_queues[i]->size = 0;
    }
}

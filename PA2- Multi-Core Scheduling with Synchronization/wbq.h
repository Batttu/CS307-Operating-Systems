#ifndef WBQ_H
#define WBQ_H

#include <pthread.h>

typedef struct Task {
    char* task_id;
    int task_duration;
    double cache_warmed_up;
    struct WorkBalancerQueue* owner;
} Task;

//   declare WorkBalancerQueue
typedef struct WorkBalancerQueue WorkBalancerQueue;

// QueueNode struct 
typedef struct QueueNode {
    Task* task;
    struct QueueNode* next;
    struct QueueNode* prev;
} QueueNode;

// WorkBalancerQueue struct
struct WorkBalancerQueue {
    QueueNode* head;            // Head of the queue
    QueueNode* tail;            // Tail of the queue
    int size;                   // Number of tasks in the queue
    pthread_mutex_t mutex;      // Mutex for synchronization
};

//this was given in document
typedef struct ThreadArguments {
    WorkBalancerQueue* q;
    int id;
} ThreadArguments;

// WorkBalancerQueue api
void submitTask(WorkBalancerQueue* q, Task* _task);
Task* fetchTask(WorkBalancerQueue* q);
Task* fetchTaskFromOthers(WorkBalancerQueue* q);

// simulator thread funcs
void executeJob(Task* task, WorkBalancerQueue* my_queue, int my_id);
void* processJobs(void* arg);
void initSharedVariables();

#endif

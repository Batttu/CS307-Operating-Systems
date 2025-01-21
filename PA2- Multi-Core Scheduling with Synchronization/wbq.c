#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "wbq.h"

// submiting task to tail of queue (owner thread)
void submitTask(WorkBalancerQueue* q, Task* _task) {
    // create new queuenode
    QueueNode* node = (QueueNode*)malloc(sizeof(QueueNode));
    node->task = _task;
    node->next = NULL;
    node->prev = NULL;

    // lock queue mutex
    pthread_mutex_lock(&(q->mutex));

    // insert node at tail of queue
    if (q->tail == NULL) {
        // empty queueu
        q->head = node;
        q->tail = node;
    } else {
        node->prev = q->tail;
        q->tail->next = node;
        q->tail = node;
    }

    // increment queue size
    q->size++;

    // unloc queue mutex
    pthread_mutex_unlock(&(q->mutex));
}

// fetch task from head of queue (owner thread
Task* fetchTask(WorkBalancerQueue* q) {
    // lock queue mutex
    pthread_mutex_lock(&(q->mutex));

    // check if queue is empty
    if (q->head == NULL) {
        pthread_mutex_unlock(&(q->mutex));
        return NULL;
    }

    // remove node from head
    QueueNode* node = q->head;
    Task* task = node->task;

    q->head = node->next;
    if (q->head != NULL) {
        q->head->prev = NULL;
    } else {
        // queue is empty here
        q->tail = NULL;
    }

    // decrement queue size
    q->size--;

    // free node
    free(node);

    // unlock queue mutex
    pthread_mutex_unlock(&(q->mutex));

    return task;
}

// fetch task from tail of another cores queue
Task* fetchTaskFromOthers(WorkBalancerQueue* q) {
    // lock queue mutex
    pthread_mutex_lock(&(q->mutex));

    // check if queue empty
    if (q->tail == NULL) {
        pthread_mutex_unlock(&(q->mutex));
        return NULL;
    }

    // remove node from tail
    QueueNode* node = q->tail;
    Task* task = node->task;

    q->tail = node->prev;
    if (q->tail != NULL) {
        q->tail->next = NULL;
    } else {
        // queue is empty here
        q->head = NULL;
    }

    // decrement queue size
    q->size--;

    // free node
    free(node);

    // unlock queue mutex
    pthread_mutex_unlock(&(q->mutex));

    return task;
}

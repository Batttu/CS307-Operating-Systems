#ifndef TOUR_H
#define TOUR_H

#include <pthread.h>
#include <semaphore.h>
#include <exception>
#include <stdexcept>
#include <stdio.h>

class Tour {
public:
    // Constructor
    Tour(int groupSize, int tourGuidePresent);
    // Destructor
    ~Tour();

    void arrive();
    void start(); // Implemented by you
    void leave();

private:
    int groupSize;          // Number of visitors needed to start a tour (excluding guide)
    int tourGuidePresent;   // 1 if guide is required, 0 otherwise
    int totalVisitorsNeeded;// Total number of people needed to start a tour
    int visitorsWaiting;    // Number of visitors currently waiting to start a tour
    int visitorsLeaving;    // Number of visitors who have left the tour
    bool tourStarted;       // True if tour has started
    pthread_t guideThreadID;// Thread ID of the tour guide

    sem_t mutex;            // Semaphore for mutual exclusion
    sem_t tourEnd;          // Semaphore to synchronize end of the tour
    sem_t arrival;          // Semaphore to block new arrivals when tour is in progress
    pthread_mutex_t printMutex; // Mutex for print statements
};

// Constructor implementation
Tour::Tour(int groupSize, int tourGuidePresent) {
    // Check arguments
    if (groupSize <= 0 || (tourGuidePresent != 0 && tourGuidePresent != 1)) {
        throw std::invalid_argument("An error occurred.");
    }

    this->groupSize = groupSize;
    this->tourGuidePresent = tourGuidePresent;
    this->totalVisitorsNeeded = groupSize + tourGuidePresent;
    this->visitorsWaiting = 0;
    this->visitorsLeaving = 0;
    this->tourStarted = false;
    this->guideThreadID = 0;

    sem_init(&mutex, 0, 1); // Mutex smeaphore starts unlocked
    sem_init(&tourEnd, 0, 0); // Tour end semaphore starts locked
    sem_init(&arrival, 0, 1); // Arrival semaphore starts unlocked

    pthread_mutex_init(&printMutex, NULL);
}

// Destructor
Tour::~Tour() {
    sem_destroy(&mutex);
    sem_destroy(&tourEnd);
    sem_destroy(&arrival);

    pthread_mutex_destroy(&printMutex);
}

// Arrive method
void Tour::arrive() {
    pthread_t tid = pthread_self();

    // Print arrival message
    pthread_mutex_lock(&printMutex);
    printf("Thread ID: %lu | Status: Arrived at the location.\n", tid);
    pthread_mutex_unlock(&printMutex);

    // Wait to enter if arrivals are blocked
    sem_wait(&arrival);
    sem_post(&arrival); // Allow others to check arrival status

    // Enter critical section
    sem_wait(&mutex);

    // Increment visitors waiting
    visitorsWaiting++;

    int currentVisitors = visitorsWaiting;

    if (!tourStarted && visitorsWaiting == totalVisitorsNeeded) {
        // Assign guide if required
        if (tourGuidePresent == 1 && guideThreadID == 0) {
            guideThreadID = tid;
        }

        // Print tour starting message
        pthread_mutex_lock(&printMutex);
        printf("Thread ID: %lu | Status: There are enough visitors, the tour is starting.\n", tid);
        pthread_mutex_unlock(&printMutex);

        // Set tour as started
        tourStarted = true;

        // Block new arrivals
        sem_wait(&arrival); // Decrease arrival semaphore to 0

        sem_post(&mutex);
    } else {
        // Not enough visitors yet
        sem_post(&mutex);

        // Print solo shots message
        pthread_mutex_lock(&printMutex);
        printf("Thread ID: %lu | Status: Only %d visitors inside, starting solo shots.\n", tid, currentVisitors);
        pthread_mutex_unlock(&printMutex);

        // Proceed to start() without waiting
    }
}

// Leave method
void Tour::leave() {
    pthread_t tid = pthread_self();

    sem_wait(&mutex);

    if (!tourStarted) {
        // Tour has not started, visitor leaves
        visitorsWaiting--;
        sem_post(&mutex);

        // Print leaving message
        pthread_mutex_lock(&printMutex);
        printf("Thread ID: %lu | Status: My camera ran out of memory while waiting, I am leaving.\n", tid);
        pthread_mutex_unlock(&printMutex);

    } else {
        sem_post(&mutex);

        if (tourGuidePresent == 1 && pthread_equal(tid, guideThreadID)) {
            // This is the guide
            // Guide announces tour is over
            pthread_mutex_lock(&printMutex);
            printf("Thread ID: %lu | Status: Tour guide speaking, the tour is over.\n", tid);
            pthread_mutex_unlock(&printMutex);

            // Release visitors waiting on tourEnd
            for (int i = 0; i < totalVisitorsNeeded - 1; i++) {
                sem_post(&tourEnd);
            }
        } else {
            // Wait for tour end if guide is present
            if (tourGuidePresent == 1) {
                sem_wait(&tourEnd);
            }

            // Print visitor leaving message
            pthread_mutex_lock(&printMutex);
            printf("Thread ID: %lu | Status: I am a visitor and I am leaving.\n", tid);
            pthread_mutex_unlock(&printMutex);
        }

        sem_wait(&mutex);
        visitorsLeaving++;

        if (visitorsLeaving == totalVisitorsNeeded) {
            // Last visitor
            // Print message
            pthread_mutex_lock(&printMutex);
            printf("Thread ID: %lu | Status: All visitors have left, the new visitors can come.\n", tid);
            pthread_mutex_unlock(&printMutex);

            // Reset state
            visitorsWaiting = 0;
            visitorsLeaving = 0;
            tourStarted = false;
            guideThreadID = 0;

            // Allow new arrivals
            sem_post(&arrival); // Increase arrival to 1
        }

        sem_post(&mutex);
    }
}
#endif // TOUR_H

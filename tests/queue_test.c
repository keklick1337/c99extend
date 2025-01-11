/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * queue_test.c
 *
 * Simple test of the FIFO queue usage in C99, now using our "adv_thread.h"
 * instead of direct pthreads.
 *
 * Demonstrates both basic and thread-safe operations.
 */

#include <stdio.h>
#include <stdlib.h>
/* Instead of <pthread.h>, include our adv_thread */
#include "adv_thread.h"
#include "queue.h"

#define NUM_THREADS 4
#define ITEMS_PER_THREAD 5

/*
 * Structure for thread arguments
 */
typedef struct {
    Queue* queue;
    int    thread_id;
} ThreadArg;

/*
 * Producer thread: pushes several integers into the queue.
 * Note: must match signature: void* func(void*).
 */
void* producer_thread(void* arg) {
    ThreadArg* p = (ThreadArg*)arg;
    Queue* q = p->queue;
    int tid = p->thread_id;

    for (int i = 0; i < ITEMS_PER_THREAD; i++) {
        int* data = (int*)malloc(sizeof(int));
        *data = tid * 100 + i; // For example, 100,101,... for thread #1
        queue_push(q, data);

        printf("[Producer %d] Pushed %d\n", tid, *data);
    }
    return NULL;
}

/*
 * Consumer thread: pops elements from the queue.
 * For demonstration, we pop a fixed number of times.
 */
void* consumer_thread(void* arg) {
    ThreadArg* p = (ThreadArg*)arg;
    Queue* q = p->queue;
    int tid = p->thread_id;

    for (int i = 0; i < ITEMS_PER_THREAD; i++) {
        // This call blocks if the queue is empty
        int* data = (int*)queue_pop(q);
        printf("[Consumer %d] Popped %d\n", tid, *data);
        free(data);
    }
    return NULL;
}

int main(void) {
    Queue* q = queue_create();
    if (!q) {
        printf("Failed to create the queue!\n");
        return 1;
    }

    /*
     * 1. Simple test without threads
     */
    int x1 = 42, x2 = 999;
    queue_push(q, &x1);
    queue_push(q, &x2);

    printf("Simple check: size = %zu\n", queue_size(q));

    int* val1 = (int*)queue_pop(q);
    int* val2 = (int*)queue_pop(q);
    printf("Popped %d and %d\n", *val1, *val2);

    /*
     * 2. Multithreaded test: multiple producers and consumers
     *    now using our AdvThread from adv_thread.h
     */
    AdvThread producers[NUM_THREADS];
    AdvThread consumers[NUM_THREADS];
    ThreadArg args[NUM_THREADS];

    // Create producer threads
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].queue = q;
        args[i].thread_id = i;
        if (thread_create(&producers[i], producer_thread, &args[i]) != 0) {
            printf("Failed to create producer thread %d\n", i);
        }
    }

    // Create consumer threads
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].queue = q;
        args[i].thread_id = i + 100; // just to distinguish
        if (thread_create(&consumers[i], consumer_thread, &args[i]) != 0) {
            printf("Failed to create consumer thread %d\n", i + 100);
        }
    }

    // Join all threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_join(&producers[i]);
        thread_join(&consumers[i]);
    }

    printf("Queue size after all threads finished: %zu\n", queue_size(q));

    queue_destroy(q);
    return 0;
}

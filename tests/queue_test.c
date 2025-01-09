/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * queue_test.c
 *
 * Simple test of the FIFO queue usage in C99.
 * Demonstrates both basic and thread-safe operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
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
     */
    pthread_t producers[NUM_THREADS];
    pthread_t consumers[NUM_THREADS];
    ThreadArg args[NUM_THREADS];

    // Create producer threads
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].queue = q;
        args[i].thread_id = i;
        pthread_create(&producers[i], NULL, producer_thread, &args[i]);
    }

    // Create consumer threads
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].queue = q;
        args[i].thread_id = i + 100; // just to distinguish
        pthread_create(&consumers[i], NULL, consumer_thread, &args[i]);
    }

    // Join all threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }

    printf("Queue size after all threads finished: %zu\n", queue_size(q));

    queue_destroy(q);
    return 0;
}

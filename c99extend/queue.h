/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * queue.h
 *
 * Header file for a simple FIFO queue in C99.
 * Provides basic operations and thread-safe usage via pthreads.
 */

#ifndef K_QUEUE_H
#define K_QUEUE_H

#include <stddef.h>     // size_t
#include <stdbool.h>    // bool
#include <pthread.h>    // pthread_* for thread-safety

/*
 * Queue node structure
 */
typedef struct QueueNode {
    void*               data;   // Pointer to user data
    struct QueueNode*   next;   // Pointer to the next node
} QueueNode;

/*
 * Main queue structure
 */
typedef struct {
    QueueNode*    head;   // Pointer to the head of the queue
    QueueNode*    tail;   // Pointer to the tail of the queue
    size_t        size;   // Current number of elements in the queue

    // Thread-safety fields
    pthread_mutex_t mutex; // Mutex for synchronization
    pthread_cond_t  cond;  // Condition variable to wait/notify
} Queue;

/*
 * Creates an empty queue and returns a pointer to it.
 */
Queue* queue_create(void);

/*
 * Destroys the queue, freeing all nodes.
 * User is responsible for freeing the actual data if necessary.
 */
void queue_destroy(Queue* q);

/*
 * Pushes a new element into the queue (FIFO).
 * 'data' is a pointer to user-allocated memory.
 */
void queue_push(Queue* q, void* data);

/*
 * Pops an element from the queue. If the queue is empty,
 * it blocks until an element becomes available.
 * Returns a pointer to the popped data.
 */
void* queue_pop(Queue* q);

/*
 * Returns true if the queue is empty (non-blocking check).
 */
bool queue_is_empty(Queue* q);

/*
 * Returns the current number of elements in the queue (non-blocking check).
 */
size_t queue_size(Queue* q);

#endif // K_QUEUE_H

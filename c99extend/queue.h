/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * queue.h
 *
 * Header file for a simple FIFO queue in C99.
 * Provides basic operations and thread-safe usage (now cross-platform).
 */

#ifndef K_QUEUE_H
#define K_QUEUE_H

#include <stddef.h>     // size_t
#include <stdbool.h>    // bool

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
    QueueNode*  head;   // Pointer to the head of the queue
    QueueNode*  tail;   // Pointer to the tail of the queue
    size_t      size;   // Current number of elements

    // Instead of pthread_mutex_t, use our cross-platform approach:
    // We'll do an adv_semaphore for usage or a small "Mutex" type.
    // For simplicity, let's do a binary semaphore as a mutex:
    // We'll do a counting semaphore of 1 => a "mutex"
    // And another semaphore for "items" if we want blocking pop.
    
    // "mutex" protects the queue structure
    // "items" is signaled when new items are available
    // "space" could also be used if we had a limit, but we skip that for now.

    // We'll just use 1 "mutex" + 1 "items".
    // For cross-platform "mutex", we can do adv_semaphore or a custom "adv_mutex".
    // Let's do a binary sem for the queue lock, and a counting sem for items.
    
    // For demonstration, we do:
    void* mutex;  // We'll store a pointer to a "binary semaphore" or "adv_mutex"
    void* items;  // a counting semaphore for number of items
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

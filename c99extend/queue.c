/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * queue.c
 *
 * Implementation of a FIFO queue in C99 with thread-safe operations.
 */

#include "queue.h"
#include <stdlib.h>

/*
 * Creates an empty queue.
 */
Queue* queue_create(void) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        return NULL; // Failed to allocate memory
    }
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;

    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);

    return q;
}

/*
 * Destroys the queue, freeing all nodes.
 * User data must be freed separately if needed.
 */
void queue_destroy(Queue* q) {
    if (!q) return;

    // Free all nodes
    QueueNode* current = q->head;
    while (current) {
        QueueNode* temp = current;
        current = current->next;
        free(temp);
    }

    // Destroy synchronization primitives
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);

    free(q);
}

/*
 * Pushes a new element to the tail of the queue.
 */
void queue_push(Queue* q, void* data) {
    if (!q) return;

    QueueNode* node = (QueueNode*)malloc(sizeof(QueueNode));
    if (!node) {
        // Failed to allocate a new node
        return;
    }
    node->data = data;
    node->next = NULL;

    pthread_mutex_lock(&q->mutex);

    if (!q->tail) {
        // Empty queue: new node is head and tail
        q->head = node;
        q->tail = node;
    } else {
        // Non-empty queue: insert at tail
        q->tail->next = node;
        q->tail = node;
    }
    q->size++;

    // Notify one waiting thread that a new element is available
    pthread_cond_signal(&q->cond);

    pthread_mutex_unlock(&q->mutex);
}

/*
 * Pops an element from the head of the queue.
 * If the queue is empty, it blocks until a new element is pushed.
 */
void* queue_pop(Queue* q) {
    if (!q) return NULL;

    pthread_mutex_lock(&q->mutex);

    // Wait until the queue is not empty
    while (q->size == 0) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    // Remove the head node
    QueueNode* node = q->head;
    void* data = node->data;

    q->head = node->next;
    if (!q->head) {
        // Queue became empty
        q->tail = NULL;
    }
    q->size--;

    free(node);

    pthread_mutex_unlock(&q->mutex);
    return data;
}

/*
 * Returns true if the queue is empty (non-blocking check).
 */
bool queue_is_empty(Queue* q) {
    if (!q) return true;

    pthread_mutex_lock(&q->mutex);
    bool empty = (q->size == 0);
    pthread_mutex_unlock(&q->mutex);

    return empty;
}

/*
 * Returns the current size of the queue (non-blocking check).
 */
size_t queue_size(Queue* q) {
    if (!q) return 0;

    pthread_mutex_lock(&q->mutex);
    size_t s = q->size;
    pthread_mutex_unlock(&q->mutex);

    return s;
}

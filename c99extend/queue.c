/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * queue.c
 *
 * Implementation of a FIFO queue in C99 with cross-platform "thread-safe" usage.
 * We use 1 binary semaphore as a mutex, and 1 counting semaphore for items.
 */

#include "queue.h"
#include "adv_semaphore.h"
#include <stdlib.h>

/*
 * We define a simple wrapper to represent "Mutex" as a binary semaphore.
 */
typedef struct {
    Semaphore sem;
} Mutex;

/*
 * Creates a binary semaphore with initial_count=1 => acts like a mutex
 */
static void mutex_init(Mutex* m) {
    // We ignore max_count or pass 1
    Semaphore_init(&m->sem, 1, 1);
}
static void mutex_destroy(Mutex* m) {
    Semaphore_destroy(&m->sem);
}
static void mutex_lock(Mutex* m) {
    Semaphore_wait(&m->sem);
}
static void mutex_unlock(Mutex* m) {
    Semaphore_post(&m->sem);
}

Queue* queue_create(void) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) return NULL;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;

    // allocate the Mutex and counting Semaphore
    Mutex* m = (Mutex*)malloc(sizeof(Mutex));
    if (!m) {
        free(q);
        return NULL;
    }
    mutex_init(m);
    q->mutex = m;

    Semaphore* item_sem = (Semaphore*)malloc(sizeof(Semaphore));
    if (!item_sem) {
        mutex_destroy(m);
        free(m);
        free(q);
        return NULL;
    }
    // Counting semaphore, starts with 0 => no items
    Semaphore_init(item_sem, 0, 9999999); // large max
    q->items = item_sem;

    return q;
}

void queue_destroy(Queue* q) {
    if (!q) return;
    // free all nodes
    QueueNode* current = q->head;
    while (current) {
        QueueNode* temp = current;
        current = current->next;
        free(temp);
    }

    // destroy semaphores
    if (q->items) {
        Semaphore* s = (Semaphore*)q->items;
        Semaphore_destroy(s);
        free(s);
    }
    if (q->mutex) {
        Mutex* m = (Mutex*)q->mutex;
        mutex_destroy(m);
        free(m);
    }
    free(q);
}

void queue_push(Queue* q, void* data) {
    if (!q) return;
    // create a new node
    QueueNode* node = (QueueNode*)malloc(sizeof(QueueNode));
    if (!node) return;
    node->data = data;
    node->next = NULL;

    // lock
    Mutex* m = (Mutex*)q->mutex;
    mutex_lock(m);

    if (!q->tail) {
        q->head = node;
        q->tail = node;
    } else {
        q->tail->next = node;
        q->tail = node;
    }
    q->size++;

    mutex_unlock(m);

    // signal that we have 1 more item
    Semaphore* s = (Semaphore*)q->items;
    Semaphore_post(s);
}

void* queue_pop(Queue* q) {
    if (!q) return NULL;
    // wait for an item to appear
    Semaphore* s = (Semaphore*)q->items;
    Semaphore_wait(s);

    // lock
    Mutex* m = (Mutex*)q->mutex;
    mutex_lock(m);

    QueueNode* node = q->head;
    void* data = node->data;
    q->head = node->next;
    if (!q->head) {
        q->tail = NULL;
    }
    q->size--;

    free(node);
    mutex_unlock(m);

    return data;
}

bool queue_is_empty(Queue* q) {
    if (!q) return true;
    Mutex* m = (Mutex*)q->mutex;
    mutex_lock(m);
    bool empty = (q->size == 0);
    mutex_unlock(m);
    return empty;
}

size_t queue_size(Queue* q) {
    if (!q) return 0;
    Mutex* m = (Mutex*)q->mutex;
    mutex_lock(m);
    size_t s = q->size;
    mutex_unlock(m);
    return s;
}

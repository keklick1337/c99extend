/*
 * test_main.c
 *
 * Example usage of:
 *   - adv_thread (class-like Thread)
 *   - adv_semaphore (simple cross-platform semaphore)
 *   - queue (thread-safe FIFO queue)
 */

#include <stdio.h>
#include <stdlib.h>
#include "adv_thread.h"
#include "adv_semaphore.h"
#include "queue.h"

/* Simple function for the thread target */
static void my_thread_func(void* arg) {
    const char* msg = (const char*)arg;
    printf("[my_thread_func] running with message: %s\n", msg);
}

int main(void) {
    printf("=== test_main ===\n");

    /* Test adv_thread: create, start, join */
    Thread t;
    Thread_init(&t, my_thread_func, "Hello from thread!", "MyThread");
    Thread_start(&t);

    printf("Thread name: %s\n", Thread_get_name(&t));
    printf("Thread is alive? %d\n", (int)Thread_is_alive(&t));

    // Let's forcibly kill if we want
    // Thread_kill(&t);

    Thread_join(&t);  // if we didn't kill, it will just join after run finishes
    printf("Thread is alive after join? %d\n", (int)Thread_is_alive(&t));

    /* Test adv_semaphore: 2 threads wait on a semaphore? 
       We'll skip for brevity, but usage is straightforward. */

    /* Test queue */
    Queue* q = queue_create();
    int x=10, y=20, z=30;
    queue_push(q, &x);
    queue_push(q, &y);
    queue_push(q, &z);
    printf("Queue size = %zu\n", queue_size(q));

    int* px = (int*)queue_pop(q);
    int* py = (int*)queue_pop(q);
    printf("Popped: %d and %d\n", *px, *py);
    printf("Queue size now = %zu\n", queue_size(q));

    queue_destroy(q);

    printf("=== end of test_main ===\n");
    return 0;
}

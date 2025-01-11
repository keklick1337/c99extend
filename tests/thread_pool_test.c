/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * thread_pool_test.c
 *
 * Demonstration of the thread pool usage in C99.
 * Compile with -pthread (e.g., gcc -std=c99 -Wall -Wextra -pthread ...)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // for sleep
#include "thread_pool.h"

/*
 * A sample task function: prints a message + sleeps for demonstration.
 */
static void sample_task(void* arg) {
    int idx = *(int*)arg;
    printf("[ThreadPool Task] Processing index: %d\n", idx);
    // Simulate some work
    sleep(1);
}

int main(void) {
    // 1. Create a thread pool with, say, 4 threads
    ThreadPool* pool = thread_pool_create(4);
    if (!pool) {
        printf("Failed to create thread pool!\n");
        return 1;
    }

    // 2. Submit several tasks
    const int NUM_TASKS = 8;
    int* data_array = (int*)malloc(NUM_TASKS * sizeof(int));
    for (int i = 0; i < NUM_TASKS; i++) {
        data_array[i] = i;
        if (!thread_pool_submit(pool, sample_task, &data_array[i])) {
            printf("Failed to submit task #%d\n", i);
        }
    }

    // 3. Destroy the pool -> waits for tasks to finish first
    thread_pool_destroy(pool);
    free(data_array);

    // All tasks should have been processed
    printf("All tasks completed, thread pool destroyed.\n");
    return 0;
}

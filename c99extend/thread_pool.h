/*
 * thread_pool.h
 * Thread pool that internally manages "AdvThread" objects.
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stdbool.h>
#include <stddef.h>

// Forward declaration
typedef struct ThreadPool ThreadPool;

// Task function signature
typedef void (*ThreadPoolTaskFn)(void* arg);

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Creates a thread pool with 'num_threads'. Returns NULL if fail.
 */
ThreadPool* thread_pool_create(size_t num_threads);

/*
 * Submits a new task. Returns false if the pool is shutting down or invalid.
 */
bool thread_pool_submit(ThreadPool* pool, ThreadPoolTaskFn fn, void* arg);

/*
 * Gracefully shuts down the pool, waiting for tasks and threads.
 */
void thread_pool_destroy(ThreadPool* pool);

#ifdef __cplusplus
}
#endif

#endif // THREAD_POOL_H

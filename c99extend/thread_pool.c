/*
 * thread_pool.c
 * Implementation of a thread pool using our "AdvThread" class,
 * so it's cross-platform (Windows, Linux, macOS, etc.).
 */

#include "thread_pool.h"
#include "adv_thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * A node for the task queue
 */
typedef struct TaskNode {
    ThreadPoolTaskFn fn;
    void*            arg;
    struct TaskNode* next;
} TaskNode;

struct ThreadPool {
    AdvThread*  workers;       // array of threads
    size_t      num_threads;
    volatile bool shutdown_flag;

    // tasks queue
    TaskNode*   task_head;
    TaskNode*   task_tail;

    // synchronization
#ifdef _WIN32
    CRITICAL_SECTION cs;
    CONDITION_VARIABLE cond;
#else
    pthread_mutex_t  mutex;
    pthread_cond_t   cond;
#endif
};

/* Forward declarations */
static void* thread_pool_worker(void* arg);

ThreadPool* thread_pool_create(size_t num_threads) {
    if (num_threads == 0) return NULL;

    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (!pool) return NULL;
    memset(pool, 0, sizeof(ThreadPool));
    pool->num_threads = num_threads;
    pool->workers = (AdvThread*)malloc(sizeof(AdvThread) * num_threads);
    if (!pool->workers) {
        free(pool);
        return NULL;
    }

    pool->shutdown_flag = false;
    pool->task_head = NULL;
    pool->task_tail = NULL;

#ifdef _WIN32
    InitializeCriticalSection(&pool->cs);
    InitializeConditionVariable(&pool->cond);
#else
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);
#endif

    // Create worker threads
    for (size_t i = 0; i < num_threads; i++) {
        if (thread_create(&pool->workers[i], thread_pool_worker, pool) != 0) {
            // creation failed => handle partial creation
            // for simplicity, ignore partial cleanup
            fprintf(stderr, "[ThreadPool] failed to create thread.\n");
        }
    }
    return pool;
}

bool thread_pool_submit(ThreadPool* pool, ThreadPoolTaskFn fn, void* arg) {
    if (!pool || !fn) return false;

    // lock
#ifdef _WIN32
    EnterCriticalSection(&pool->cs);
#else
    pthread_mutex_lock(&pool->mutex);
#endif

    if (pool->shutdown_flag) {
#ifdef _WIN32
        LeaveCriticalSection(&pool->cs);
#else
        pthread_mutex_unlock(&pool->mutex);
#endif
        return false;
    }
    // create new task
    TaskNode* node = (TaskNode*)malloc(sizeof(TaskNode));
    if (!node) {
#ifdef _WIN32
        LeaveCriticalSection(&pool->cs);
#else
        pthread_mutex_unlock(&pool->mutex);
#endif
        return false;
    }
    node->fn = fn;
    node->arg = arg;
    node->next = NULL;

    // enqueue
    if (pool->task_tail) {
        pool->task_tail->next = node;
        pool->task_tail = node;
    } else {
        pool->task_head = node;
        pool->task_tail = node;
    }

    // notify a worker
#ifdef _WIN32
    WakeConditionVariable(&pool->cond);
    LeaveCriticalSection(&pool->cs);
#else
    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
#endif

    return true;
}

void thread_pool_destroy(ThreadPool* pool) {
    if (!pool) return;

    // signal shutdown
#ifdef _WIN32
    EnterCriticalSection(&pool->cs);
    pool->shutdown_flag = true;
    WakeAllConditionVariable(&pool->cond);
    LeaveCriticalSection(&pool->cs);
#else
    pthread_mutex_lock(&pool->mutex);
    pool->shutdown_flag = true;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
#endif

    // join all
    for (size_t i = 0; i < pool->num_threads; i++) {
        thread_join(&pool->workers[i]);
    }

    // free tasks
    TaskNode* cur = pool->task_head;
    while (cur) {
        TaskNode* tmp = cur;
        cur = cur->next;
        free(tmp);
    }

    free(pool->workers);

#ifdef _WIN32
    DeleteCriticalSection(&pool->cs);
#else
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
#endif

    free(pool);
}

static void* thread_pool_worker(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    if (!pool) return NULL;

    for (;;) {
        // lock
#ifdef _WIN32
        EnterCriticalSection(&pool->cs);
        while (!pool->shutdown_flag && pool->task_head == NULL) {
            SleepConditionVariableCS(&pool->cond, &pool->cs, INFINITE);
        }
        if (pool->shutdown_flag && pool->task_head == NULL) {
            LeaveCriticalSection(&pool->cs);
            break;
        }
        TaskNode* task = pool->task_head;
        pool->task_head = task->next;
        if (!pool->task_head) {
            pool->task_tail = NULL;
        }
        LeaveCriticalSection(&pool->cs);
#else
        pthread_mutex_lock(&pool->mutex);
        while (!pool->shutdown_flag && pool->task_head == NULL) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        if (pool->shutdown_flag && pool->task_head == NULL) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        TaskNode* task = pool->task_head;
        pool->task_head = task->next;
        if (!pool->task_head) {
            pool->task_tail = NULL;
        }
        pthread_mutex_unlock(&pool->mutex);
#endif

        // run the task
        task->fn(task->arg);
        free(task);
    }

    return NULL;
}

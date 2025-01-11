/*
 * adv_thread.c
 *
 * Implementation of cross-platform Thread + the new thread_create/thread_join.
 */

#include "adv_thread.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
static DWORD WINAPI _thread_bootstrap(LPVOID lpParam);
#else
static void* _thread_bootstrap(void* arg);
#endif

/* ========== Old "class-like" Thread methods ========== */

void Thread_init(Thread* t, void (*target)(void*), void* arg, const char* name) {
    if (!t) return;
    t->started = false;
    t->joined  = false;
    t->killed  = false;
    t->is_alive= false;
    t->target  = target;
    t->arg     = arg;
    if (name) {
#ifdef _WIN32
        strncpy_s(t->name, sizeof(t->name), name, _TRUNCATE);
#else
        snprintf(t->name, sizeof(t->name), "%s", name);
#endif
    } else {
#ifdef _WIN32
        strncpy_s(t->name, sizeof(t->name), "Thread", _TRUNCATE);
#else
        snprintf(t->name, sizeof(t->name), "%s", "Thread");
#endif
    }
#ifdef _WIN32
    t->handle   = NULL;
    t->thread_id= 0;
#else
    t->thread_id= 0;
#endif
}

void Thread_run(Thread* t) {
    if (!t) return;
    // default calls target if not NULL
    if (t->target) {
        t->target(t->arg);
    }
}

#ifdef _WIN32
static DWORD WINAPI _thread_bootstrap(LPVOID lpParam) {
    Thread* t = (Thread*)lpParam;
    if (!t) return 0;
    t->is_alive = true;
    Thread_run(t);
    t->is_alive = false;
    return 0;
}
#else
static void* _thread_bootstrap(void* arg) {
    Thread* t = (Thread*)arg;
    if (!t) return NULL;
    t->is_alive = true;
    Thread_run(t);
    t->is_alive = false;
    return NULL;
}
#endif

void Thread_start(Thread* t) {
    if (!t || t->started) return;
    t->started = true;
#ifdef _WIN32
    DWORD tid;
    HANDLE h = CreateThread(NULL, 0, _thread_bootstrap, t, 0, &tid);
    if (h != NULL) {
        t->handle = h;
        t->thread_id = tid;
    } else {
        t->started = false; // failed
    }
#else
    int rc = pthread_create(&t->thread_id, NULL, _thread_bootstrap, t);
    if (rc != 0) {
        t->started = false; // failed
    }
#endif
}

void Thread_join(Thread* t) {
    if (!t || !t->started || t->joined) return;
    t->joined = true;
#ifdef _WIN32
    WaitForSingleObject(t->handle, INFINITE);
    CloseHandle(t->handle);
    t->handle = NULL;
#else
    pthread_join(t->thread_id, NULL);
#endif
}

void Thread_kill(Thread* t) {
    if (!t || !t->started || t->killed) return;
    t->killed = true;
#ifdef _WIN32
    if (t->handle) {
        TerminateThread(t->handle, 0);
        CloseHandle(t->handle);
        t->handle = NULL;
    }
#else
    pthread_kill(t->thread_id, SIGKILL);
#endif
    t->is_alive = false;
}

bool Thread_is_alive(Thread* t) {
    if (!t) return false;
    return t->is_alive;
}

void Thread_set_name(Thread* t, const char* name) {
    if (!t || t->started) return;
    if (name) {
#ifdef _WIN32
        strncpy_s(t->name, sizeof(t->name), name, _TRUNCATE);
#else
        snprintf(t->name, sizeof(t->name), "%s", name);
#endif
    }
}

const char* Thread_get_name(Thread* t) {
    if (!t) return "";
    return t->name;
}

/* ========== New functions required by thread_pool.c ========== */

/*
 * thread_create(AdvThread* t, ...) => returns int
 */
int thread_create(AdvThread* t, void *(*start_routine)(void*), void* arg) {
    if (!t || !start_routine) {
        return -1;
    }
    // We'll do the same logic as Thread_init + Thread_start, 
    // but we store the function in t->target as a cast from (void*)(void*) => There's a mismatch in signature:
    //   old: t->target is type: void(*)(void*)
    //   new: we pass a start_routine that returns void*, e.g. "thread_pool_worker".
    // We'll do a small trampoline or just cast.
    t->target = (void(*)(void*))start_routine;
    t->arg    = arg;
    Thread_init(t, t->target, t->arg, NULL); // or set name to something
    // Now start:
    Thread_start(t);
    if (!t->started) {
        return -1;
    }
    return 0;
}

/*
 * thread_join(AdvThread* t) => returns int
 */
int thread_join(AdvThread* t) {
    if (!t) return -1;
    if (!t->started || t->joined) {
        return 0; // or -1 if you want
    }
    Thread_join(t);
    return 0;
}

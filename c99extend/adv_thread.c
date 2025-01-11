/*
 * adv_thread.c
 *
 * Implementation of cross-platform Thread + the new thread_create/thread_join.
 */

#include "adv_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
static DWORD WINAPI _thread_bootstrap(LPVOID lpParam);
#else
static void* _thread_bootstrap(void* arg);
#endif

/*
 * We'll store a small struct to hold the "void*(*)(void*)" function pointer
 * plus its argument, so we can call it inside Thread_run without mismatched casts.
 */
typedef struct {
    void *(*start_routine)(void*);
    void* arg;
} ThreadTrampoline;

/* ========== Old "class-like" Thread methods ========== */

/*
 * Initialize the Thread object with a "classic" target (returning void).
 */
void Thread_init(Thread* t, void (*target)(void*), void* arg, const char* name) {
    if (!t) return;
    t->started   = false;
    t->joined    = false;
    t->killed    = false;
    t->is_alive  = false;
    t->target    = target;
    t->arg       = arg;

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
    t->handle    = NULL;
    t->thread_id = 0;
#else
    t->thread_id = 0;
#endif
}

/*
 * Actually run the thread: if 'target' is the normal function,
 * we just call it. If 'target' is our special trampoline function,
 * we handle that scenario differently.
 */
void Thread_run(Thread* t) {
    if (!t) return;
    if (t->target) {
        /* If we detect that 't->target' is our 'trampoline' function,
           we cast 't->arg' to ThreadTrampoline, call start_routine, free it, etc.
         */
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
 * We create a small static function that matches 'void(*)(void*)' type
 * but inside it, we safely call the user function returning void*.
 */

/* The real trampoline function that we store in 't->target'. */
static void _thread_routine_trampoline(void* arg) {
    ThreadTrampoline* td = (ThreadTrampoline*)arg;
    if (!td) return;
    /* call the user-provided start_routine (which returns void*) */
    (void) td->start_routine(td->arg); 
    /* optional: we could store the return value somewhere if needed. */

    /* free the trampoline data */
    free(td);
}

/*
 * thread_create(AdvThread* t, ...) => returns int
 */
int thread_create(AdvThread* t, void *(*start_routine)(void*), void* arg) {
    if (!t || !start_routine) {
        return -1;
    }
    /* Allocate a small block that holds the user function and its arg. */
    ThreadTrampoline* td = (ThreadTrampoline*)malloc(sizeof(ThreadTrampoline));
    if (!td) {
        return -1;
    }
    td->start_routine = start_routine;
    td->arg           = arg;

    /* Instead of casting the function pointer, we set t->target
       to our known-compatible function '_thread_routine_trampoline' (which returns void).
       Then we set t->arg to the 'td' block.
    */
    Thread_init(t, _thread_routine_trampoline, td, NULL);

    /* Now start */
    Thread_start(t);
    if (!t->started) {
        free(td);
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

/*
 * adv_thread.h
 *
 * Provides a "Thread" structure in C99, plus a convenient alias "AdvThread"
 * so that old code referencing "AdvThread" still works.
 *
 * We also add "thread_create" and "thread_join" that return int for error handling.
 *
 * Supported: Windows (_WIN32) or POSIX (Linux/BSD/macOS).
 */

#ifndef ADV_THREAD_H
#define ADV_THREAD_H

#include <stdbool.h>

#ifdef _WIN32
  #include <windows.h>
#elif defined(__unix__) || defined(__APPLE__) || defined(__MACH__) || defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  #include <pthread.h>
  #include <signal.h>
  #include <string.h>
#else
  #error "Unsupported platform for adv_thread!"
#endif

/*
 * Declaration of the Thread struct
 */
typedef struct Thread {
#ifdef _WIN32
    HANDLE     handle;
    unsigned   thread_id;
#else
    pthread_t  thread_id;
#endif
    bool       started;
    bool       joined;
    bool       killed;
    bool       is_alive;
    char       name[64];

    /* user-specified "target" function (like in the old adv_thread) */
    void     (*target)(void*);
    void*      arg;
} Thread;

/* Provide an alias: "AdvThread" is the same as "Thread" */
typedef Thread AdvThread;

/*
 * Optional older methods from previous code:
 * (like a "class" in Python style)
 */
void Thread_init(Thread* t, void (*target)(void*), void* arg, const char* name);
void Thread_start(Thread* t);
void Thread_run(Thread* t);
void Thread_join(Thread* t);   /* returns void in old style */
void Thread_kill(Thread* t);
bool Thread_is_alive(Thread* t);
void Thread_set_name(Thread* t, const char* name);
const char* Thread_get_name(Thread* t);

/*
 * New API for code that expects "thread_create" returning int, "thread_join" returning int
 * plus using "AdvThread" type.
 *
 * If everything is fine => returns 0. If fails => return -1.
 */
int thread_create(AdvThread* t, void *(*start_routine)(void*), void* arg);
int thread_join(AdvThread* t);

#endif // ADV_THREAD_H

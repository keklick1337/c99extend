/*
 * adv_semaphore.c
 */

#include "adv_semaphore.h"
#include <stdio.h>

#if defined(_WIN32)
/* ===================== Windows ===================== */
bool Semaphore_init(Semaphore* s, unsigned int initial_count, unsigned int max_count) {
    if (!s) return false;
    s->handle = CreateSemaphore(NULL, (LONG)initial_count, (LONG)max_count, NULL);
    if (s->handle == NULL) {
        return false;
    }
    return true;
}

void Semaphore_destroy(Semaphore* s) {
    if (!s) return;
    if (s->handle) {
        CloseHandle(s->handle);
        s->handle = NULL;
    }
}

void Semaphore_wait(Semaphore* s) {
    if (!s) return;
    WaitForSingleObject(s->handle, INFINITE);
}

void Semaphore_post(Semaphore* s) {
    if (!s) return;
    ReleaseSemaphore(s->handle, 1, NULL);
}

/* ===================== macOS (GCD) ===================== */
#elif defined(__APPLE__)
#include <limits.h> // for INT_MAX if needed

bool Semaphore_init(Semaphore* s, unsigned int initial_count, unsigned int max_count) {
    if (!s) return false;
    // For GCD, we only have dispatch_semaphore_create(long value).
    // If initial_count > LONG_MAX => we saturate
    long initVal = (long)((initial_count > 0x7fffffff) ? 0x7fffffff : initial_count);
    s->sem = dispatch_semaphore_create(initVal);
    if (!s->sem) {
        return false;
    }
    (void)max_count; // unused
    return true;
}

void Semaphore_destroy(Semaphore* s) {
    // dispatch semaphores are reference-counted objects.
    // The recommended approach in pure C is just to do nothing,
    // or handle with _Nonnull bridging if in ObjC. 
    // We'll do nothing for now. If you want, we can do a CFRetain/CFRelease dance.
    (void)s;
}

void Semaphore_wait(Semaphore* s) {
    if (!s || !s->sem) return;
    dispatch_semaphore_wait(s->sem, DISPATCH_TIME_FOREVER);
}

void Semaphore_post(Semaphore* s) {
    if (!s || !s->sem) return;
    dispatch_semaphore_signal(s->sem);
}

/* ===================== POSIX (Linux/BSD) ===================== */
#elif defined(__unix__) || defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

#include <semaphore.h>
#include <errno.h>

bool Semaphore_init(Semaphore* s, unsigned int initial_count, unsigned int max_count) {
    if (!s) return false;
    // max_count is not used in sem_init; it's basically unlimited or at least 32767.
    // We ignore it.
    if (sem_init(&s->sem, 0, initial_count) != 0) {
        return false;
    }
    (void)max_count;
    return true;
}

void Semaphore_destroy(Semaphore* s) {
    if (!s) return;
    sem_destroy(&s->sem);
}

void Semaphore_wait(Semaphore* s) {
    if (!s) return;
    sem_wait(&s->sem);
}

void Semaphore_post(Semaphore* s) {
    if (!s) return;
    sem_post(&s->sem);
}

#else
#error "Unsupported platform for adv_semaphore!"
#endif

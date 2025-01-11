/*
 * adv_semaphore.h
 *
 * Cross-platform semaphore in C99.
 * On Windows, uses CreateSemaphore / WaitForSingleObject / ReleaseSemaphore.
 * On Linux/BSD, uses sem_init / sem_wait / sem_post (POSIX).
 * On Apple (macOS), we switch to dispatch_semaphore if we want to avoid deprecated sem_init.
 */

#ifndef ADV_SEMAPHORE_H
#define ADV_SEMAPHORE_H

#include <stdbool.h>
#include <stddef.h>

#if defined(_WIN32)

  #include <windows.h>
  typedef struct Semaphore {
      HANDLE handle;
  } Semaphore;

#elif defined(__APPLE__)
  // use GCD semaphores
  #include <dispatch/dispatch.h>
  typedef struct Semaphore {
      dispatch_semaphore_t sem;
  } Semaphore;

#elif defined(__unix__) || defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  // "normal" POSIX semaphores
  #include <semaphore.h>
  typedef struct Semaphore {
      sem_t sem;
  } Semaphore;

#else
  #error "Unsupported platform for adv_semaphore!"
#endif

/*
 * Initialize the semaphore with an initial count and an optional max_count.
 * On macOS with GCD, max_count is ignored; on Windows, it is used in CreateSemaphore.
 * On POSIX, we don't have a direct max_count, so we ignore it as well.
 *
 * Returns true on success, false on failure.
 */
bool Semaphore_init(Semaphore* s, unsigned int initial_count, unsigned int max_count);

/*
 * Destroy the semaphore.
 */
void Semaphore_destroy(Semaphore* s);

/*
 * Wait (decrement). Blocks if count is zero.
 */
void Semaphore_wait(Semaphore* s);

/*
 * Post (increment).
 */
void Semaphore_post(Semaphore* s);

#endif // ADV_SEMAPHORE_H

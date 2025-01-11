# **Project Documentation**

**Author**: Vladislav Tislenko aka keklick1337 (2025)  
**Language**: C99 (strict)  
**Description**: This project provides multiple C99 headers for extended functionality:
1. **string_utf8.h**  
2. **thread_pool.h**  
3. **adv_thread.h**  
4. **adv_semaphore.h**  
5. **containers.h**  
6. **queue.h**  

Below is an overview of each header, the main data structures, and the primary functions they export.

---

## 1) `string_utf8.h`

**Location**: `./c99extend/string_utf8.h`

**Purpose**:  
Implements an extended UTF-8 string library in C99, providing:

- **`String` structure**:  
  ```c
  typedef struct {
      char*  data;      
      size_t len_bytes; 
      size_t len_utf8;  
      size_t cap;       
  } String;
  ```
  - `data`: pointer to the character buffer (dynamically allocated).  
  - `len_bytes`: current length in bytes (not counting the `'\0'`).  
  - `len_utf8`: number of UTF-8 code points currently stored.  
  - `cap`: allocated capacity in bytes (including space for `'\0'`).  

- **Basic Functions**:
  - `String str_init(void)`: creates an empty `String`.  
  - `String str_from_cstr(const char* cstr)`: creates a `String` from a regular C-string.  
  - `void str_free(String* s)`: frees the internal buffer.  
  - `const char* str_data(const String* s)`: returns a `const char*` pointer for reading.  
  - `void str_push_back(String* s, char c)`: appends one character.  
  - `void str_concat(String* dest, const String* src)`: concatenates two `String`s in-place.  
  - `String str_plus(const String* s1, const String* s2)`: returns a new `String` = `s1 + s2`.  
  - `void str_reserve(String* s, size_t new_cap)`: reserves more capacity.  

- **UTF-8 Validation**:
  - `bool utf8_validate(const char* data, size_t length)`: checks raw data for valid UTF-8 sequences.  
  - `bool str_validate_utf8(const String* s)`: same check for a `String`.  
  - `bool str_preflight_utf8(String* s)`: checks validity, possibly prints warnings.  

- **BOM Handling**:
  - `bool str_remove_utf8_bom(String* s)`: removes the UTF-8 BOM if present (`0xEF 0xBB 0xBF`).  

- **CRLF / LF**:
  - `void str_strip_crlf(String* s)`: removes trailing `\r` or `\n` from the end.

---

## 2) `thread_pool.h`

**Location**: `./c99extend/thread_pool.h`

**Purpose**:  
Implements a simple thread pool that uses **`AdvThread`** under the hood (see `adv_thread.h`) and provides a user-friendly API for submitting tasks.

- **Forward Declaration**:  
  ```c
  typedef struct ThreadPool ThreadPool;
  ```
- **Task Function**:  
  ```c
  typedef void (*ThreadPoolTaskFn)(void* arg);
  ```
- **Functions**:
  - `ThreadPool* thread_pool_create(size_t num_threads)`: creates a pool with `num_threads`.  
  - `bool thread_pool_submit(ThreadPool* pool, ThreadPoolTaskFn fn, void* arg)`: submits a task.  
  - `void thread_pool_destroy(ThreadPool* pool)`: shuts down the pool gracefully.

> **Important**: The header currently contains an `extern "C"` block, which is a C++-ism, so in strict C99 you would remove or comment it out if you want purely C code.

---

## 3) `adv_thread.h`

**Location**: `./c99extend/adv_thread.h`

**Purpose**:  
Provides a cross-platform “Thread” abstraction for either **Windows** or **POSIX**. The code merges `pthread_t` usage (on POSIX) with `HANDLE` usage (on Windows). It also defines a fallback or `#error` for unsupported platforms.

**Key Points**:

- **`Thread` structure**:  
  ```c
  typedef struct Thread {
      #ifdef _WIN32
          HANDLE    handle;
          unsigned  thread_id;
      #elif ...
          pthread_t thread_id;
      #endif
      bool started, joined, killed, is_alive;
      char name[64];
      void (*target)(void*);
      void* arg;
  } Thread;

  typedef Thread AdvThread;
  ```
- **Functions**:
  - `void Thread_init(Thread* t, void (*target)(void*), void* arg, const char* name);`  
  - `void Thread_start(Thread* t);`  
  - `void Thread_run(Thread* t);`  
  - `void Thread_join(Thread* t);`  
  - `void Thread_kill(Thread* t);`  
  - `bool Thread_is_alive(Thread* t);`  
  - `void Thread_set_name(Thread* t, const char* name);`  
  - `const char* Thread_get_name(Thread* t);`  
  - `int thread_create(AdvThread* t, void *(*start_routine)(void*), void* arg);`  
  - `int thread_join(AdvThread* t);`  

The last two (`thread_create`, `thread_join`) are helper functions returning an `int` error code (0 = success, -1 = fail).

> **Note**: In your code snippet, there seems to be conflicting declarations (both `HANDLE thread_id;` and `pthread_t thread_id;` inside the same struct). Typically you'd do `#ifdef _WIN32` vs. `#else` blocks to avoid redefinition.

---

## 4) `adv_semaphore.h`

**Location**: `./c99extend/adv_semaphore.h`

**Purpose**:  
A cross-platform semaphore in C99. Uses:

- **Windows**: `CreateSemaphore`, `WaitForSingleObject`, `ReleaseSemaphore`.  
- **macOS** (Apple): `dispatch_semaphore_t` to avoid deprecated `sem_init`.  
- **Linux/BSD**: `sem_init`, `sem_wait`, `sem_post`.  

**Possible Structures** (depending on `#ifdef`):
```c
typedef struct Semaphore {
    #ifdef _WIN32
       HANDLE handle;
    #elif defined(__APPLE__)
       dispatch_semaphore_t sem;
    #elif ...
       sem_t sem;
    #endif
} Semaphore;
```

**Functions**:
- `bool Semaphore_init(Semaphore* s, unsigned int initial_count, unsigned int max_count);`
- `void Semaphore_destroy(Semaphore* s);`
- `void Semaphore_wait(Semaphore* s);`
- `void Semaphore_post(Semaphore* s);`

> **Note**: In the snippet you pasted, there's no explicit `#ifdef _WIN32 ... #elif ... #else`, but rather multiple includes in a row with `#error`. Make sure your real code uses proper conditionals so each platform sees only one definition.

---

## 5) `containers.h`

**Location**: `./c99extend/containers.h`

**Purpose**:  
Provides several container data structures in C99:

### 5.1 Dynamic Array
```c
typedef struct {
    void** data;
    size_t size;
    size_t capacity;
} DynArray;
```
- **`da_create`**: allocate a new array.  
- **`da_push_back`**: append an element (amortized O(1)).  
- **`da_pop_back`**: remove the last element.  
- **`da_destroy`**: free the structure (but not the items).  
- Higher-order: `da_map`, `da_filter`, `da_reduce`.

### 5.2 Hash Table (string -> void*)
```c
typedef struct HashTable HashTable;
```
- **`ht_create`**: create with a given capacity.  
- **`ht_insert`**: store or update key->value.  
- **`ht_get`**: retrieve value.  
- **`ht_remove`**: remove entry.  
- **`ht_destroy`**: free the entire table.

### 5.3 Red-Black Tree (int -> void*)
```c
typedef struct RBTree RBTree;
```
- **`rbt_create`**, `rbt_insert`, `rbt_find`, `rbt_remove`, `rbt_destroy`.

### 5.4 Generic HashSet
```c
typedef size_t (*HS_HashFn)(const void*);
typedef bool   (*HS_EqFn)(const void*, const void*);
typedef struct HashSet HashSet;
```
- **`hs_create(size_t initial_capacity, HS_HashFn hashFn, HS_EqFn eqFn)`**  
- **`hs_insert`**: inserts a new element (duplicates are ignored).  
- **`hs_contains`**: checks membership.  
- **`hs_remove`**: removes an element.  
- **`hs_iterate`**: calls a user callback for each element in no particular order.  
- **`hs_destroy`**: frees internal structures (does not free user data pointers).

---

## 6) `queue.h`

**Location**: `./c99extend/queue.h`

**Purpose**:  
Implements a simple FIFO queue (thread-safe) using cross-platform semaphores or locks.

- **`QueueNode`**:
  ```c
  typedef struct QueueNode {
      void* data;
      struct QueueNode* next;
  } QueueNode;
  ```
- **`Queue`**:
  ```c
  typedef struct {
      QueueNode* head;
      QueueNode* tail;
      size_t size;
      void* mutex;
      void* items;
  } Queue;
  ```
  - `mutex` is typically a binary semaphore or other cross-platform mechanism.  
  - `items` is a counting semaphore indicating how many items are in the queue.  

- **Functions**:
  - `queue_create()`: allocates a new queue, initializes semaphores.  
  - `queue_destroy()`: frees all nodes, semaphores.  
  - `queue_push()`: push an element (FIFO).  
  - `queue_pop()`: pop the oldest element (blocks if empty).  
  - `queue_is_empty()`: returns true if size == 0 (non-blocking).  
  - `queue_size()`: returns the current number of elements.

---

## Additional Notes

- **Strict C99**: All headers should compile under `-std=c99 -Wall -Wextra -Werror -pedantic` with proper platform checks (`#ifdef _WIN32`, `#elif defined(__linux__) ...`, etc.).  
- **Threading**: 
  - `thread_pool.h` depends on `adv_thread.h` to actually spawn worker threads.  
  - `adv_thread.h` merges Windows (HANDLE) and POSIX (pthread_t). Make sure the final code in `.c` files uses `#ifdef` to avoid redefinition.  
- **Semaphore**: 
  - `adv_semaphore.h` merges Windows (HANDLE), Apple GCD dispatch semaphores, and POSIX semaphores. Carefully handle them with `#ifdef __APPLE__` or `_WIN32`.

---

## Quick Reference

**Build** example (Linux/macOS):
```bash
gcc -std=c99 -Wall -Wextra -Werror -pedantic -O2 \ 
    c99extend/string_utf8.c c99extend/thread_pool.c c99extend/adv_thread.c c99extend/adv_semaphore.c \
    c99extend/containers.c c99extend/queue.c \
    tests/any_test.c \
    -o any_test -pthread
```
Then run `./any_test`.


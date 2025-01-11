# c99extend

**Extended C99 Library (Threads, Queues, UTF-8 Strings, Containers)**  
Author: [Vladislav Tislenko aka keklick1337](https://github.com/keklick1337)  
Year: 2025  

[![Open Documentation](https://img.shields.io/badge/Click%20for%20DOC-Reference-orange)](DOC.md)

This repository provides an **extended C99 library** that offers:

- **Thread Pool and Thread Abstraction** (using `adv_thread.h` / `thread_pool.h`)
- **Semaphore** (cross-platform, in `adv_semaphore.h`)
- **Thread-safe Queue** (`queue.h` / `queue.c`)
- **Enhanced UTF-8 String** library (`string_utf8.h` / `string_utf8.c`)
- **Miscellaneous Data Structures** (`containers.h` / `containers.c`):
  - Dynamic Array
  - Hash Table
  - Red-Black Tree
  - Generic HashSet  

Everything is written in pure C99, aiming to simplify common data-structure management, reliable UTF-8 handling, and cross-platform threading primitives.

---

## What’s Included

1. **Thread Abstractions (`adv_thread.h`)**  
   - Cross-platform `Thread` (Windows `_WIN32` or POSIX).  
   - Provides `thread_create`, `thread_join`, `Thread_kill`, etc.

2. **Thread Pool (`thread_pool.h`)**  
   - Worker threads managed internally.  
   - Simple API: create pool, submit tasks, destroy when done.  

3. **Semaphore (`adv_semaphore.h`)**  
   - Cross-platform: uses Windows `CreateSemaphore`, Apple GCD dispatch semaphores, or POSIX semaphores.  

4. **Queues (`queue.h` / `queue.c`)**  
   - Thread-safe FIFO queue.  
   - Multiple producers/consumers can safely push/pop data.  

5. **UTF-8 Strings (`string_utf8.h` / `string_utf8.c`)**  
   - Manages dynamic strings with byte-length and UTF-8 codepoint count.  
   - Validates UTF-8, strips BOM, handles CRLF, etc.  

6. **Additional Containers (`containers.h` / `containers.c`)**  
   - **Dynamic Array**  
   - **Hash Table** (string -> `void*`)  
   - **Red-Black Tree** (int -> `void*`)  
   - **Generic HashSet** (supporting custom hash/equality)

---

## Repository Structure

```text
.
├── LICENSE                # MIT License
├── configure              # Script to detect compiler & generate Makefile
├── README.md              # This README
├── DOC.md                 # Detailed documentation / reference
├── c99extend/
│   ├── adv_semaphore.c
│   ├── adv_semaphore.h    # Cross-platform semaphore
│   ├── adv_thread.c
│   ├── adv_thread.h       # Cross-platform Thread abstraction (POSIX/Win)
│   ├── containers.c
│   ├── containers.h       # Additional data structures (DynArray, HashTable, etc.)
│   ├── queue.c
│   ├── queue.h            # Thread-safe FIFO queue
│   ├── string_utf8.c
│   ├── string_utf8.h      # UTF-8 string library header
│   ├── thread_pool.c
│   ├── thread_pool.h      # Thread pool interface
├── tests/
│   ├── containers_test.c  # Test code for containers
│   ├── queue_test.c       # Test code for queue usage
│   ├── string_utf8_test.c # Test code for the UTF-8 string library
│   ├── test_main.c        # Test code for threads and queue usage
│   └── thread_pool_test.c # Test code for thread pool
└── test_files/
    ├── test_utf8_bom.txt   # UTF-8 text file with BOM
    └── test_utf8_nobom.txt # UTF-8 text file without BOM
```

**Note**: After running the `configure` script, a `Makefile` is generated at the root level for convenient building.

---

## Quick Start

1. **Clone the repository**  
   ```bash
   git clone https://github.com/keklick1337/c99extend.git
   cd c99extend
   ```

2. **Configure**  
   ```bash
   ./configure
   ```
   Detects a suitable compiler (Clang/GCC) and prepares a `Makefile`.

3. **Build**  
   ```bash
   make
   ```
   - Produces the static library **`libc99extend.a`**.  
   - Builds test executables (e.g. `queue_test`, `string_utf8_test`, `containers_test`) unless excluded.

4. **Run Tests**  
   ```bash
   make run
   ```
   or run individual binaries like:
   ```bash
   ./testbin/queue_test
   ./testbin/string_utf8_test
   ./testbin/containers_test
   ```
   Demonstrates:
   - Queue usage (thread-safe push/pop).  
   - UTF-8 string operations (BOM handling, invalid bytes, etc.).  
   - Additional data structures like dynamic array, hash table, or hash set.

5. **Clean**  
   ```bash
   make clean
   ```
   Removes compiled objects, the static library, test executables, and the generated `Makefile`.

---

## Using c99extend in Your Project

1. **Include** the relevant headers/source or use the static library.  
2. **Compile/Link**:  
   ```bash
   gcc -std=c99 main.c queue.c string_utf8.c -o my_app
   ```
   or:
   ```bash
   gcc -std=c99 main.c -Lc99extend -lc99extend -o my_app
   ```
   (Ensure `-pthread` if needed for threading.)

3. **Examples**:  
   - **Queue**  
     ```c
     #include "queue.h"

     int main(void) {
         Queue* q = queue_create();
         int x = 42;
         queue_push(q, &x);
         int* popped = (int*)queue_pop(q);
         // ...
         queue_destroy(q);
         return 0;
     }
     ```
   - **UTF-8 String**  
     ```c
     #include "string_utf8.h"

     int main(void) {
         String s = str_from_cstr("Hello, UTF-8!");
         if (str_preflight_utf8(&s)) {
             printf("Valid: '%s'\n", str_data(&s));
         }
         str_free(&s);
         return 0;
     }
     ```
   - **Thread Pool**  
     ```c
     #include "thread_pool.h"

     static void my_task(void* arg) {
         int num = *(int*)arg;
         printf("Task %d\n", num);
     }

     int main(void) {
         ThreadPool* pool = thread_pool_create(4);
         int x = 100;
         thread_pool_submit(pool, my_task, &x);
         // ...
         thread_pool_destroy(pool);
         return 0;
     }
     ```

---

## Documentation

Click the badge below to view the **[DOC.md](DOC.md)** file, which provides a more in-depth explanation of each header, data structure, and API.

[![DOC Reference](https://img.shields.io/badge/DOC-Reference-blue)](DOC.md)

---

## License

This project is released under the [MIT License](LICENSE).  
You are free to use, modify, and distribute this software under the conditions described in the license file.

---

## Author & Credits

- **Author**: Vladislav Tislenko (keklick1337)  
- **Year**: 2025  

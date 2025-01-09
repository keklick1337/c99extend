# c99extend

**Extended C99 Library (Queues + UTF-8 Strings)**  
Author: [Vladislav Tislenko aka keklick1337](https://github.com/keklick1337)  
Year: 2025  

This repository provides an **extended C99 library** that offers both a **thread-safe queue** implementation and an **enhanced UTF-8 string** library. Written in pure C99, it aims to simplify common data-structure management and provide reliable UTF-8 handling.  

---

## What’s Included

1. **Queues (queue.c / queue.h)**  
   - Thread-safe FIFO queue implementation.  
   - Supports multiple producers and consumers.  
   - Simple to integrate: create, push data, pop data, and destroy.  

2. **UTF-8 Strings (string_utf8.c / string_utf8.h)**  
   - Dynamic string management with byte-length (`len_bytes`) and UTF-8 codepoint count (`len_utf8`).  
   - Validation for proper UTF-8 (detects invalid bytes, overlong encodings, etc.).  
   - BOM detection and removal.  
   - CRLF stripping and other basic operations (push-back, concatenation, etc.).  

---

## Repository Structure

```text
.
├── LICENSE                # MIT License
├── configure              # Script to detect compiler & generate Makefile
├── README.md              # This README
├── c99extend/
│   ├── queue.c           # Queue implementation
│   ├── queue.h           # Queue header
│   ├── string_utf8.c     # UTF-8 string library implementation
│   └── string_utf8.h     # UTF-8 string library header
├── tests/
│   ├── queue_test.c      # Test code for queue usage
│   └── string_utf8_test.c# Test code for the UTF-8 string library
└── test_files/
    ├── test_utf8_bom.txt   # UTF-8 file with BOM
    └── test_utf8_nobom.txt # UTF-8 file without BOM
```

After running the `configure` script, a `Makefile` is generated at the root level for convenient building.

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
   - Builds the test executables **`queue_test`** and **`string_utf8_test`** (unless excluded).

4. **Run Tests**  
   ```bash
   make run
   ```
   or
   ```bash
   ./queue_test
   ./string_utf8_test
   ```
   Demonstrates:
   - Queue usage (push/pop, thread-safety example if adapted).  
   - UTF-8 string operations: BOM removal, codepoint counting, invalid byte detection, etc.

5. **Clean**  
   ```bash
   make clean
   ```
   Removes build outputs (objects, library, tests) and the generated `Makefile`.

---

## Using c99extend in Your Project

1. **Include the library**  
   - Either copy `queue.h/queue.c` and `string_utf8.h/string_utf8.c` into your project.  
   - Or link against `libc99extend.a`.

2. **Usage Examples**  
   **Queue**  
   ```c
   #include "queue.h"

   int main(void) {
       Queue* q = queue_create();
       int x = 42;
       queue_push(q, &x);
       int* popped = (int*)queue_pop(q);
       queue_destroy(q);
       return 0;
   }
   ```

   **UTF-8 String**  
   ```c
   #include "string_utf8.h"

   int main(void) {
       String s = str_from_cstr("Hello, World!");
       if (str_preflight_utf8(&s)) {
           printf("'%s' is valid UTF-8\n", str_data(&s));
       }
       str_free(&s);
       return 0;
   }
   ```

3. **Compile/Link**  
   ```bash
   gcc -std=c99 main.c queue.c string_utf8.c -o my_app
   ```
   or, if using the static library:
   ```bash
   gcc -std=c99 main.c -L. -lc99extend -o my_app
   ```

---

## License

This project is released under the [MIT License](LICENSE), allowing free use, modification, and distribution under its conditions.

---

## Author & Credits

- **Author**: Vladislav Tislenko (keklick1337)  
- **Year**: 2025  

Happy coding with **c99extend**!
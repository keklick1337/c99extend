/*
 * containers.h
 *
 * A collection of extended containers in C99:
 *   1) Dynamic Array
 *   2) Hash Table (string -> void*)
 *   3) Red-Black Tree (int -> void*)
 *   4) HashSet (set of strings)
 * plus higher-order functions (map, filter, reduce) for the dynamic array.
 *
 * by Vladislav Tislenko aka keklick1337 (2025)
 */

#ifndef C99EXT_CONTAINERS_H
#define C99EXT_CONTAINERS_H

#include <stddef.h>  /* size_t */
#include <stdbool.h> /* bool */

/* ---------------------------------------------------------
 * 1) DYNAMIC ARRAY
 * --------------------------------------------------------- */
typedef struct {
    void** data;
    size_t size;
    size_t capacity;
} DynArray;

DynArray* da_create(void);
bool      da_push_back(DynArray* arr, void* elem);
void*     da_pop_back(DynArray* arr);
void      da_destroy(DynArray* arr);

static inline size_t da_size(const DynArray* arr) { return arr ? arr->size : 0; }
static inline bool   da_empty(const DynArray* arr) { return !arr || arr->size == 0; }

/* Higher-order functions for DynArray */
typedef void* (*MapFn)(void* elem);
DynArray* da_map(const DynArray* arr, MapFn fn);

typedef bool (*FilterFn)(void* elem);
DynArray* da_filter(const DynArray* arr, FilterFn pred);

typedef void* (*ReduceFn)(void* acc, void* elem);
void* da_reduce(const DynArray* arr, void* init, ReduceFn fn);

/* ---------------------------------------------------------
 * 2) HASH TABLE (string -> void*)
 * --------------------------------------------------------- */
typedef struct HashTable HashTable;

HashTable* ht_create(size_t capacity);
bool       ht_insert(HashTable* ht, const char* key, void* value);
void*      ht_get(const HashTable* ht, const char* key);
void*      ht_remove(HashTable* ht, const char* key);
void       ht_destroy(HashTable* ht);

/* ---------------------------------------------------------
 * 3) RED-BLACK TREE (int -> void*)
 * --------------------------------------------------------- */
typedef struct RBTree RBTree;

RBTree* rbt_create(void);
bool    rbt_insert(RBTree* tree, int key, void* value);
void*   rbt_find(const RBTree* tree, int key);
void*   rbt_remove(RBTree* tree, int key);
void    rbt_destroy(RBTree* tree);


/* ========================================
 * Generic HashSet
 * ========================================
 *
 * The user provides:
 *   - hashFn:    size_t hash(const void* elem)
 *   - eqFn:      bool eq(const void* a, const void* b)
 *
 * The HashSet stores unique elements (void*) based on these functions.
 */

/* Function pointer types for hash and equality */
typedef size_t (*HS_HashFn)(const void*);
typedef bool   (*HS_EqFn)(const void*, const void*);

typedef struct HashSet HashSet;

/*
 * Creates a new HashSet with given initial capacity and user-provided hash/eq.
 * On success returns a pointer, or NULL if allocation fails.
 */
HashSet* hs_create(size_t initial_capacity,
                   HS_HashFn hashFn,
                   HS_EqFn   eqFn);

/*
 * Inserts 'elem' into the set (duplicates are no-ops).
 * Returns true if insertion succeeds or if the element already in the set,
 * false if out-of-memory or other error.
 */
bool hs_insert(HashSet* set, void* elem);

/*
 * Checks if 'elem' is in the set.
 * Returns true if found, false otherwise.
 */
bool hs_contains(const HashSet* set, const void* elem);

/*
 * Removes 'elem' from the set if present. Returns true if found+removed, false otherwise.
 */
bool hs_remove(HashSet* set, const void* elem);

/*
 * Iterates over all elements in the set (in no particular order),
 * calling fn(elem, userData) once per element.
 */
typedef void (*HS_IterFn)(void* elem, void* userData);
void hs_iterate(const HashSet* set, HS_IterFn fn, void* userData);

/*
 * Destroys the entire set. This does NOT free the user-stored pointers themselves.
 * Caller is responsible for freeing them if needed.
 */
void hs_destroy(HashSet* set);


#endif /* C99EXT_CONTAINERS_H */

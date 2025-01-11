/*
 * containers_test.c
 *
 * Minimal test for the extended containers:
 *   - Dynamic Array (with map/filter/reduce)
 *   - Hash Table
 *   - Red-Black Tree
 *
 * by Vladislav Tislenko aka keklick1337 (2025)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers.h"

/* For testing map/filter/reduce, let's define some sample functions: */

/* map: convert an integer pointer to another int pointer with incremented value */
static void* map_incr(void* elem) {
    int* old_val = (int*)elem;
    int* new_val = (int*)malloc(sizeof(int));
    *new_val = *old_val + 1;
    return new_val;
}

/* filter: keep only even integers */
static bool filter_even(void* elem) {
    int val = *(int*)elem;
    return (val % 2 == 0);
}

/* reduce: sum up all integers, storing in an int* accumulator */
static void* reduce_sum(void* acc, void* elem) {
    int* acc_val = (int*)acc;
    int* e_val   = (int*)elem;
    *acc_val += *e_val;
    return acc_val; /* same pointer, updated */
}

/* ===================== UTILS: hash/eq for strings, int, long ===================== */

static size_t str_hash(const void* ptr) {
    /* djb2, for example */
    const char* s = (const char*)ptr;
    unsigned long h = 5381;
    int c;
    while ((c = *s++)) {
        h = ((h << 5) + h) + c;
    }
    return (size_t)h;
}

static bool str_eq(const void* a, const void* b) {
    const char* sa = (const char*)a;
    const char* sb = (const char*)b;
    return (strcmp(sa, sb) == 0);
}

/* For integer, we store pointer to int => cast back to int. */
static size_t int_hash(const void* ptr) {
    int val = *(const int*)ptr;
    /* a simple mixing function */
    return (size_t)(val * 2654435761u);
}

static bool int_eq(const void* a, const void* b) {
    int aa = *(const int*)a;
    int bb = *(const int*)b;
    return (aa == bb);
}

/* For long */
static size_t long_hash(const void* ptr) {
    long val = *(const long*)ptr;
    /* let's do a basic approach. This might not be perfect */
    return (size_t)((val ^ (val >> 32)) * 2654435761u);
}

static bool long_eq(const void* a, const void* b) {
    long aa = *(const long*)a;
    long bb = *(const long*)b;
    return (aa == bb);
}

/* A helper function to print all elements in a set (assuming they're int*) */
static void print_int_elem(void* elem, void* userData) {
    (void)userData;
    int val = *(int*)elem;
    printf("%d ", val);
}

/* Similarly for strings */
static void print_str_elem(void* elem, void* userData) {
    (void)userData;
    const char* s = (const char*)elem;
    printf("'%s' ", s);
}

int main(void) {
    printf("=== containers_test ===\n\n");

    /* 1) Test Dynamic Array + map/filter/reduce */
    DynArray* arr = da_create();
    if (!arr) {
        printf("Cannot create dynamic array!\n");
        return 1;
    }

    /* push some ints */
    for (int i = 0; i < 5; i++) {
        int* p = (int*)malloc(sizeof(int));
        *p = i;
        da_push_back(arr, p);
    }
    printf("DynArray has size %zu\n", da_size(arr));

    /* da_map */
    DynArray* arr_incr = da_map(arr, map_incr);
    if (arr_incr) {
        printf("After map_incr:\n");
        for (size_t i = 0; i < da_size(arr_incr); i++) {
            int val = *((int*)arr_incr->data[i]);
            printf("  %d\n", val);
        }
    }

    /* da_filter */
    DynArray* arr_even = da_filter(arr_incr, filter_even);
    if (arr_even) {
        printf("Even elements after increment:\n");
        for (size_t i = 0; i < da_size(arr_even); i++) {
            int val = *((int*)arr_even->data[i]);
            printf("  %d\n", val);
        }
    }

    /* da_reduce: sum */
    int acc_init = 0;
    int* result = (int*)da_reduce(arr_incr, &acc_init, reduce_sum);
    printf("Sum of arr_incr = %d\n", *result);

    /* Cleanup dynamic array memory */
    /* We only free the array structures, not the data inside? Let's free them properly. */
    for (size_t i = 0; i < da_size(arr_incr); i++) {
        free(arr_incr->data[i]);
    }
    da_destroy(arr_incr);

    for (size_t i = 0; i < da_size(arr_even); i++) {
        /* These are the same pointers as arr_incr subset, but we've already freed them above,
           so be careful. In real code, we'd avoid double-free or handle references properly. */
    }
    da_destroy(arr_even);

    for (size_t i = 0; i < da_size(arr); i++) {
        free(arr->data[i]);
    }
    da_destroy(arr);

    printf("\n--- Testing HashTable ---\n");
    HashTable* ht = ht_create(8);
    ht_insert(ht, "apple", "red");
    ht_insert(ht, "banana", "yellow");
    ht_insert(ht, "grape", "purple");
    ht_insert(ht, "apple", "green");
    printf("apple -> %s\n", (char*)ht_get(ht, "apple"));
    printf("banana -> %s\n", (char*)ht_get(ht, "banana"));
    void* removed = ht_remove(ht, "banana");
    printf("Removed banana -> %s\n", (char*)removed);
    printf("banana -> %s\n", (char*)ht_get(ht, "banana")); /* should be NULL */
    ht_destroy(ht);

    printf("\n--- Testing RBTree ---\n");
    RBTree* tree = rbt_create();
    rbt_insert(tree, 10, "val10");
    rbt_insert(tree, 5,  "val5");
    rbt_insert(tree, 20, "val20");
    rbt_insert(tree, 15, "val15");
    printf("Find key=10 => %s\n", (char*)rbt_find(tree, 10));
    printf("Find key=15 => %s\n", (char*)rbt_find(tree, 15));
    void* vdel = rbt_remove(tree, 10);
    printf("Remove key=10 => %s\n", (char*)vdel);
    printf("Find key=10 => %s\n", (char*)rbt_find(tree, 10)); /* now NULL */

    rbt_destroy(tree);

    printf("\n");

    printf("=== containers_test for HashSet ===\n\n");

    /* 1) Testing an int-based HashSet */
    HashSet* intSet = hs_create(8, int_hash, int_eq);
    if (!intSet) {
        printf("Failed to create intSet.\n");
        return 1;
    }

    /* Insert some ints */
    int nums[] = {10, 20, 30, 20, 10, 40, 50, 60, 10, 20, 10, 99};
    size_t i;
    for (i = 0; i < sizeof(nums)/sizeof(nums[0]); i++) {
        hs_insert(intSet, &nums[i]);
    }

    /* Print them via iteration */
    printf("intSet elements: ");
    hs_iterate(intSet, print_int_elem, NULL);
    printf("\n");

    /* Check duplicates, removal */
    int x = 20;
    printf("Contains 20? %d\n", (int)hs_contains(intSet, &x));
    hs_remove(intSet, &x);
    printf("After remove(20): contains 20? %d\n", (int)hs_contains(intSet, &x));

    printf("intSet elements after remove(20): ");
    hs_iterate(intSet, print_int_elem, NULL);
    printf("\n");

    hs_destroy(intSet);

    /* 2) Testing a string-based HashSet */
    HashSet* strSet = hs_create(8, str_hash, str_eq);
    if (!strSet) {
        printf("Failed to create strSet.\n");
        return 1;
    }

    /* Insert some strings (we store pointers to string literals or dynamic strings) */
    const char* fruits[] = {"apple", "banana", "banana", "orange", "grape", "apple"};
    for (i = 0; i < sizeof(fruits)/sizeof(fruits[0]); i++) {
        hs_insert(strSet, (void*)fruits[i]);
    }

    printf("strSet elements: ");
    hs_iterate(strSet, print_str_elem, NULL);
    printf("\n");

    /* Remove 'banana' */
    printf("Contains 'banana'? %d\n", (int)hs_contains(strSet, "banana"));
    hs_remove(strSet, "banana");
    printf("After remove('banana'): contains 'banana'? %d\n", (int)hs_contains(strSet, "banana"));

    printf("strSet elements after removal: ");
    hs_iterate(strSet, print_str_elem, NULL);
    printf("\n");

    hs_destroy(strSet);

    /* 3) Testing a long-based HashSet (just a small example) */
    HashSet* longSet = hs_create(4, long_hash, long_eq);
    if (!longSet) {
        printf("Failed to create longSet.\n");
        return 1;
    }

    long arrLong[] = {100L, 5000000000L, -10L, 100L, 7000000000L};
    for (i = 0; i < sizeof(arrLong)/sizeof(arrLong[0]); i++) {
        hs_insert(longSet, &arrLong[i]);
    }

    /* We'll just check one long. */
    long valCheck = 5000000000L;
    printf("Contains 5000000000L? %d\n", (int)hs_contains(longSet, &valCheck));

    hs_destroy(longSet);

    printf("\n=== End of containers_test ===\n");
    return 0;
}

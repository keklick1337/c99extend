/*
 * containers.c
 *
 * Implementation of:
 *  - DynArray (dynamic array)
 *  - HashTable (string -> void* map)
 *  - RBTree (int -> void* map)
 *  - HashSet (set of strings)
 * plus higher-order functions (map/filter/reduce) for DynArray.
 *
 * by Vladislav Tislenko aka keklick1337 (2025)
 */

#include "containers.h"
#include "string_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================
 *             1) DYNAMIC ARRAY
 * ========================================================= */

DynArray* da_create(void) {
    DynArray* arr = (DynArray*)malloc(sizeof(DynArray));
    if (!arr) return NULL;
    arr->size = 0;
    arr->capacity = 4;
    arr->data = (void**)malloc(sizeof(void*) * arr->capacity);
    if (!arr->data) {
        free(arr);
        return NULL;
    }
    return arr;
}

bool da_push_back(DynArray* arr, void* elem) {
    if (!arr) return false;
    if (arr->size >= arr->capacity) {
        size_t newcap = arr->capacity * 2;
        void** tmp = (void**)realloc(arr->data, sizeof(void*) * newcap);
        if (!tmp) {
            return false;
        }
        arr->data = tmp;
        arr->capacity = newcap;
    }
    arr->data[arr->size++] = elem;
    return true;
}

void* da_pop_back(DynArray* arr) {
    if (!arr || arr->size == 0) return NULL;
    arr->size--;
    return arr->data[arr->size];
}

void da_destroy(DynArray* arr) {
    if (!arr) return;
    free(arr->data);
    free(arr);
}

/* Higher-order: map, filter, reduce */

DynArray* da_map(const DynArray* arr, MapFn fn) {
    if (!arr || !fn) return NULL;
    DynArray* out = da_create();
    if (!out) return NULL;
    size_t i;
    for (i = 0; i < arr->size; i++) {
        void* transformed = fn(arr->data[i]);
        if (!da_push_back(out, transformed)) {
            da_destroy(out);
            return NULL; /* out of memory */
        }
    }
    return out;
}

DynArray* da_filter(const DynArray* arr, FilterFn pred) {
    if (!arr || !pred) return NULL;
    DynArray* out = da_create();
    if (!out) return NULL;
    size_t i;
    for (i = 0; i < arr->size; i++) {
        if (pred(arr->data[i])) {
            if (!da_push_back(out, arr->data[i])) {
                da_destroy(out);
                return NULL;
            }
        }
    }
    return out;
}

void* da_reduce(const DynArray* arr, void* init, ReduceFn fn) {
    if (!arr || !fn) return init;
    void* acc = init;
    size_t i;
    for (i = 0; i < arr->size; i++) {
        acc = fn(acc, arr->data[i]);
    }
    return acc;
}

/* =========================================================
 *             2) HASH TABLE (string -> void*)
 * ========================================================= */

/* We'll do open addressing with linear probing. */
typedef struct {
    char* key;
    void* value;
    bool  in_use;
} HTSlot;

struct HashTable {
    HTSlot* slots;
    size_t  capacity;
    size_t  count;
};

/* a basic string hash, e.g. djb2 */
static unsigned long _strhash(const char* s) {
    unsigned long h = 5381;
    int c;
    while ((c = *s++)) {
        h = ((h << 5) + h) + c; /* h * 33 + c */
    }
    return h;
}

HashTable* ht_create(size_t capacity) {
    if (capacity < 4) capacity = 4;
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) return NULL;
    ht->slots = (HTSlot*)calloc(capacity, sizeof(HTSlot));
    if (!ht->slots) {
        free(ht);
        return NULL;
    }
    ht->capacity = capacity;
    ht->count = 0;
    return ht;
}

bool ht_insert(HashTable* ht, const char* key, void* value) {
    if (!ht || !key) return false;
    if (ht->count >= ht->capacity / 2) {
        /* maybe resize? for simplicity, skip or partial approach */
    }
    unsigned long h = _strhash(key);
    size_t idx = (size_t)(h % ht->capacity);
    size_t i;
    for (i = 0; i < ht->capacity; i++) {
        size_t probe = (idx + i) % ht->capacity;
        if (!ht->slots[probe].in_use) {
            /* insert */
            char* dup = adv_strdup(key);
            if (!dup) return false;
            ht->slots[probe].key = dup;
            ht->slots[probe].value = value;
            ht->slots[probe].in_use = true;
            ht->count++;
            return true;
        } else if (strcmp(ht->slots[probe].key, key) == 0) {
            /* update */
            ht->slots[probe].value = value;
            return true;
        }
    }
    return false;
}

void* ht_get(const HashTable* ht, const char* key) {
    if (!ht || !key) return NULL;
    unsigned long h = _strhash(key);
    size_t idx = (size_t)(h % ht->capacity);
    size_t i;
    for (i = 0; i < ht->capacity; i++) {
        size_t probe = (idx + i) % ht->capacity;
        if (!ht->slots[probe].in_use) {
            return NULL;
        }
        if (ht->slots[probe].key && strcmp(ht->slots[probe].key, key) == 0) {
            return ht->slots[probe].value;
        }
    }
    return NULL;
}

void* ht_remove(HashTable* ht, const char* key) {
    if (!ht || !key) return NULL;
    unsigned long h = _strhash(key);
    size_t idx = (size_t)(h % ht->capacity);
    size_t i;
    for (i = 0; i < ht->capacity; i++) {
        size_t probe = (idx + i) % ht->capacity;
        if (!ht->slots[probe].in_use) {
            return NULL;
        }
        if (ht->slots[probe].key && strcmp(ht->slots[probe].key, key) == 0) {
            void* val = ht->slots[probe].value;
            free(ht->slots[probe].key);
            ht->slots[probe].key = NULL;
            ht->slots[probe].value = NULL;
            ht->slots[probe].in_use = false;
            ht->count--;
            return val;
        }
    }
    return NULL;
}

void ht_destroy(HashTable* ht) {
    if (!ht) return;
    size_t i;
    for (i = 0; i < ht->capacity; i++) {
        if (ht->slots[i].in_use) {
            free(ht->slots[i].key);
        }
    }
    free(ht->slots);
    free(ht);
}

/* =========================================================
 *             3) RED-BLACK TREE (int -> void*)
 * ========================================================= */

typedef enum { RED, BLACK } RBColor;
typedef struct RBNode {
    int key;
    void* value;
    RBColor color;
    struct RBNode* parent;
    struct RBNode* left;
    struct RBNode* right;
} RBNode;

struct RBTree {
    RBNode* root;
};

/* forward declarations for internal routines */
static void rbt_insert_fixup(RBTree* tree, RBNode* node);
static RBNode* rbt_minimum(RBNode* n);

/* helper rotations */
static void rbt_left_rotate(RBTree* tree, RBNode* x);
static void rbt_right_rotate(RBTree* tree, RBNode* y);

RBTree* rbt_create(void) {
    RBTree* t = (RBTree*)malloc(sizeof(RBTree));
    if (!t) return NULL;
    t->root = NULL;
    return t;
}

static RBNode* rbt_minimum(RBNode* n) {
    while (n->left) {
        n = n->left;
    }
    return n;
}

/* left rotate utility */
static void rbt_left_rotate(RBTree* tree, RBNode* x) {
    RBNode* y = x->right;
    x->right = y->left;
    if (y->left) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (!x->parent) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

/* right rotate utility */
static void rbt_right_rotate(RBTree* tree, RBNode* y) {
    RBNode* x = y->left;
    y->left = x->right;
    if (x->right) {
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (!y->parent) {
        tree->root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }
    x->right = y;
    y->parent = x;
}

bool rbt_insert(RBTree* tree, int key, void* value) {
    if (!tree) return false;
    RBNode* node = (RBNode*)malloc(sizeof(RBNode));
    if (!node) return false;
    node->key = key;
    node->value = value;
    node->color = RED;
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;

    /* normal BST insert */
    RBNode* y = NULL;
    RBNode* x = tree->root;
    while (x) {
        y = x;
        if (key < x->key) {
            x = x->left;
        } else if (key > x->key) {
            x = x->right;
        } else {
            /* update existing key => no strict duplicates for simplicity */
            x->value = value;
            free(node);
            return true;
        }
    }
    node->parent = y;
    if (!y) {
        tree->root = node;
    } else if (node->key < y->key) {
        y->left = node;
    } else {
        y->right = node;
    }

    /* fixup */
    rbt_insert_fixup(tree, node);
    return true;
}

static void rbt_insert_fixup(RBTree* tree, RBNode* z) {
    while (z->parent && z->parent->color == RED) {
        RBNode* gp = z->parent->parent;
        if (z->parent == gp->left) {
            RBNode* uncle = gp->right;
            if (uncle && uncle->color == RED) {
                z->parent->color = BLACK;
                uncle->color = BLACK;
                gp->color = RED;
                z = gp;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rbt_left_rotate(tree, z);
                }
                z->parent->color = BLACK;
                gp->color = RED;
                rbt_right_rotate(tree, gp);
            }
        } else {
            RBNode* uncle = gp->left;
            if (uncle && uncle->color == RED) {
                z->parent->color = BLACK;
                uncle->color = BLACK;
                gp->color = RED;
                z = gp;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rbt_right_rotate(tree, z);
                }
                z->parent->color = BLACK;
                gp->color = RED;
                rbt_left_rotate(tree, gp);
            }
        }
    }
    if (tree->root) {
        tree->root->color = BLACK;
    }
}

void* rbt_find(const RBTree* tree, int key) {
    if (!tree) return NULL;
    RBNode* x = tree->root;
    while (x) {
        if (key < x->key) {
            x = x->left;
        } else if (key > x->key) {
            x = x->right;
        } else {
            return x->value;
        }
    }
    return NULL;
}

void* rbt_remove(RBTree* tree, int key) {
    if (!tree) return NULL;
    RBNode* node = tree->root;
    while (node) {
        if (key < node->key) {
            node = node->left;
        } else if (key > node->key) {
            node = node->right;
        } else {
            void* val = node->value;
            /* If both children are non-null, swap with successor. */
            if (node->left && node->right) {
                RBNode* succ = rbt_minimum(node->right);
                node->key = succ->key;
                node->value = succ->value;
                node = succ; /* we'll remove 'succ' instead. */
            }
            RBNode* child = (node->left) ? node->left : node->right;
            if (child) {
                child->parent = node->parent;
            }
            if (!node->parent) {
                tree->root = child;
            } else if (node == node->parent->left) {
                node->parent->left = child;
            } else {
                node->parent->right = child;
            }
            free(node);
            /* skipping fixup for brevity */
            return val;
        }
    }
    return NULL;
}

/* post-order traversal to free nodes */
/* small recursive function: */
void _free_subtree(RBNode* n) {
    if (!n) return;
    _free_subtree(n->left);
    _free_subtree(n->right);
    free(n);
}

void rbt_destroy(RBTree* tree) {
    if (!tree) return;

    _free_subtree(tree->root);
    free(tree);
}


/* =========================================================
 *             GENERIC HASHSET
 * ========================================================= */

typedef enum {
    SLOT_EMPTY = 0,  /* never used */
    SLOT_FILLED = 1, /* in use */
    SLOT_REMOVED= 2  /* was in use, then removed => tombstone */
} SlotState;

/*
 * Each slot holds:
 *   - data (void*)
 *   - state (empty, filled, removed)
 *   - cached hash
 */
typedef struct {
    void*     data;
    SlotState state;
    size_t    hash;
} HS_Slot;

struct HashSet {
    HS_Slot*  slots;
    size_t    capacity;
    size_t    size;      /* number of FILLED items */
    HS_HashFn hashFn;
    HS_EqFn   eqFn;

    float     maxLoadFactor; /* e.g. 0.75 */
};

/* forward declarations */
static bool hs_resize(HashSet* set, size_t newCap);

/*
 * Creates a new HashSet with given capacity, hashFn, eqFn.
 */
HashSet* hs_create(size_t initial_capacity,
                   HS_HashFn hashFn,
                   HS_EqFn   eqFn)
{
    if (initial_capacity < 4) {
        initial_capacity = 4;
    }
    HashSet* hs = (HashSet*)malloc(sizeof(HashSet));
    if (!hs) return NULL;

    hs->slots = (HS_Slot*)calloc(initial_capacity, sizeof(HS_Slot));
    if (!hs->slots) {
        free(hs);
        return NULL;
    }
    hs->capacity = initial_capacity;
    hs->size = 0;
    hs->hashFn = hashFn;
    hs->eqFn   = eqFn;
    hs->maxLoadFactor = 0.75f; /* default load factor */

    return hs;
}

/*
 * Insert or do nothing if element is already in the set.
 */
bool hs_insert(HashSet* set, void* elem) {
    if (!set || !elem) return false;
    /* check load factor */
    float load = (float)set->size / (float)set->capacity;
    if (load > set->maxLoadFactor) {
        /* try to resize if possible */
        size_t newCap = set->capacity * 2;
        if (!hs_resize(set, newCap)) {
            /* if resizing fails, we can try to continue anyway, but collisions might skyrocket. */
        }
    }
    size_t h = set->hashFn(elem);
    size_t index = h % set->capacity;
    size_t i;
    size_t firstRemovedIndex = (size_t)-1; /* track first tombstone slot if found */

    for (i = 0; i < set->capacity; i++) {
        size_t probe = (index + i) % set->capacity;
        HS_Slot* slot = &set->slots[probe];

        if (slot->state == SLOT_EMPTY) {
            /* use either this slot or previously found tombstone */
            size_t finalPos = (firstRemovedIndex == (size_t)-1) ? probe : firstRemovedIndex;
            HS_Slot* finalSlot = &set->slots[finalPos];
            finalSlot->data = elem;
            finalSlot->hash = h;
            finalSlot->state= SLOT_FILLED;
            set->size++;
            return true;
        }
        else if (slot->state == SLOT_REMOVED) {
            /* record the tombstone if we haven't yet */
            if (firstRemovedIndex == (size_t)-1) {
                firstRemovedIndex = probe;
            }
        }
        else if (slot->state == SLOT_FILLED) {
            /* check if it's the same element */
            if (slot->hash == h && set->eqFn(slot->data, elem)) {
                /* already in the set => no-op */
                return true;
            }
        }
    }
    /* table is full (or highly colliding) */
    return false;
}

/*
 * Contains
 */
bool hs_contains(const HashSet* set, const void* elem) {
    if (!set || !elem) return false;
    size_t h = set->hashFn(elem);
    size_t index = h % set->capacity;
    size_t i;
    for (i = 0; i < set->capacity; i++) {
        size_t probe = (index + i) % set->capacity;
        const HS_Slot* slot = &set->slots[probe];
        if (slot->state == SLOT_EMPTY) {
            /* can't be further in open addressing => not found */
            return false;
        }
        else if (slot->state == SLOT_FILLED) {
            if (slot->hash == h && set->eqFn(slot->data, elem)) {
                return true;
            }
        }
        /* if SLOT_REMOVED, we keep searching */
    }
    return false;
}

/*
 * Remove
 */
bool hs_remove(HashSet* set, const void* elem) {
    if (!set || !elem) return false;
    size_t h = set->hashFn(elem);
    size_t index = h % set->capacity;
    size_t i;
    for (i = 0; i < set->capacity; i++) {
        size_t probe = (index + i) % set->capacity;
        HS_Slot* slot = &set->slots[probe];
        if (slot->state == SLOT_EMPTY) {
            /* not found */
            return false;
        }
        else if (slot->state == SLOT_FILLED) {
            if (slot->hash == h && set->eqFn(slot->data, elem)) {
                /* found => mark removed */
                slot->state = SLOT_REMOVED;
                slot->data  = NULL; /* user must free separately if needed */
                set->size--;
                return true;
            }
        }
    }
    return false;
}

/*
 * Iterate
 */
void hs_iterate(const HashSet* set, HS_IterFn fn, void* userData) {
    if (!set || !fn) return;
    size_t i;
    for (i = 0; i < set->capacity; i++) {
        const HS_Slot* slot = &set->slots[i];
        if (slot->state == SLOT_FILLED) {
            fn(slot->data, userData);
        }
    }
}

/*
 * Destroy
 */
void hs_destroy(HashSet* set) {
    if (!set) return;
    free(set->slots);
    free(set);
}

/*
 * Optionally, resizing function if we want the set to grow when load factor is exceeded.
 */
static bool hs_resize(HashSet* set, size_t newCap) {
    HS_Slot* newSlots = (HS_Slot*)calloc(newCap, sizeof(HS_Slot));
    if (!newSlots) {
        return false;
    }
    /* re-insert from old array */
    HS_Slot* oldSlots = set->slots;
    size_t oldCap = set->capacity;

    set->slots = newSlots;
    set->capacity = newCap;
    set->size = 0; /* will re-count as we insert */

    size_t i;
    for (i = 0; i < oldCap; i++) {
        if (oldSlots[i].state == SLOT_FILLED) {
            /* re-insert */
            void* elem = oldSlots[i].data;
            size_t h = oldSlots[i].hash;
            /* do normal insertion logic, but we skip eq check since we know it's unique. */
            size_t index = h % set->capacity;
            size_t j;
            size_t firstRemovedIdx = (size_t)-1;
            for (j = 0; j < set->capacity; j++) {
                size_t probe = (index + j) % set->capacity;
                HS_Slot* slot = &set->slots[probe];
                if (slot->state == SLOT_EMPTY) {
                    size_t finalP = (firstRemovedIdx == (size_t)-1) ? probe : firstRemovedIdx;
                    HS_Slot* fs = &set->slots[finalP];
                    fs->data = elem;
                    fs->hash = h;
                    fs->state= SLOT_FILLED;
                    set->size++;
                    break;
                }
                else if (slot->state == SLOT_REMOVED) {
                    if (firstRemovedIdx == (size_t)-1) {
                        firstRemovedIdx = probe;
                    }
                }
            }
        }
    }
    free(oldSlots);
    return true;
}

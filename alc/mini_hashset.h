#pragma once

#include <inttypes.h>
#include "diagnostics.h"
#include "typesys.h"

// hash entry

typedef struct {
    str key;
    reg_t value;
} hash_entry;

bool entry_valid(const str *entry) {
    return entry->data;
}

void entry_invalidate(str *entry) {
    entry->data = NULL;
}

// mini hashset

#define array_len ('Z' - 'A' + 1)

typedef hash_entry mini_hashset[array_len];

u64 hash(str id) {
    u64 hash = 0xcbf29ce484222325;
    while (id.data != id.end) {
        hash ^= (u64)*id.data++;
        hash *= 0x100000001b3;
    }
    return hash;
}

inline static hash_entry *find_entry(mini_hashset self, const str id) {
    u64 index = hash(id) % array_len;
    str_printdnl(id), printd(" -> hash was: %"PRIu64"\n", index);
    u64 start = index;

    while (entry_valid(&self[index].key)) {
        if (str_eq(self[index].key, id)) {
            break;
        }
        index += 1;
        index %= array_len;
        if (index == start) {
            fprintf(stderr, "hash table is full: entry");
            abort();
        }
    }
    return &self[index];
}

inline static bool has_entry(mini_hashset self, const str *s) {
    hash_entry *entry = find_entry(self, *s);
    return entry_valid(&entry->key);
}

inline static reg_t *overwrite_id(mini_hashset self, str id, const reg_t *value) {
    hash_entry *entry = find_entry(self, id);
    entry->key = id, entry->value = *value;
    return &entry->value;
}

inline static reg_t *add_id(mini_hashset self, str id, const reg_t *value) {
    hash_entry *entry = find_entry(self, id);
    if (entry_valid(&entry->key)) {
        return NULL;
    } else {
        entry->key = id, entry->value = *value;
    }
    return &entry->value;
}

// arr mini hashset

#define MAX_DEPTH 7

typedef struct {
    mini_hashset data[MAX_DEPTH];
    mini_hashset *cur;
} arr_mini_hashset;

arr_mini_hashset local_ids;

static inline void mini_hashset_clear(mini_hashset self) {
    for (int i = 0; i < array_len; ++i) {
        entry_invalidate(&self[i].key);
    }
}
static inline void arr_mini_hashset_init(arr_mini_hashset *arr) {
    arr->cur = arr->data;
    mini_hashset_clear(*arr->cur);
}
static inline mini_hashset *arr_mini_hashset_push(arr_mini_hashset *arr) {
    if (arr->cur + 1 == arr->data + MAX_DEPTH) {
        fputs("arr mini hashset was full\n", stderr);
        abort();
    }
    arr->cur++;
    mini_hashset_clear(*arr->cur);
    printd("array is now %zd\n", arr->cur - arr->data);
    return arr->cur - 1;
}
static inline void arr_mini_hashset_pop(arr_mini_hashset *arr) {
    if (arr->cur == arr->data)
        return;
    arr->cur--;
    printd("array is now %zd\n", arr->cur - arr->data);
}
static inline mini_hashset *arr_mini_hashset_top(arr_mini_hashset *arr) {
    if (arr->cur == arr->data)
        return 0;
    return arr->cur - 1;
}

inline static bool find_id(arr_mini_hashset *arr, str id, reg_t **out) {
    for (mini_hashset *scope = arr->cur; scope >= arr->data; --scope) {
        hash_entry *entry = find_entry(*scope, id);
        if (entry_valid(&entry->key)) {
            *out = &entry->value;
            return true;
        }
    }
    *out = &find_entry(*arr->cur, id)->value;
    return false;
}

// constant entry

typedef struct const_entry {
    i64 value;
} const_entry_t;

#define CONST_MINI_HASHSET_SIZE 64

typedef struct const_hashmap {
    str keys[CONST_MINI_HASHSET_SIZE];
    const_entry_t values[CONST_MINI_HASHSET_SIZE];
} const_hashmap_t;

inline static u64 const_hashmap_find_index(const_hashmap_t *self, str id) {
    u64 index = hash(id) % array_len;
    const u64 start = index;

    while (entry_valid(&self->keys[index])) {
        if (str_eq(self->keys[index], id)) {
            break;
        }
        index += 1;
        index &= (array_len - 1);
        if (index == start) {
            fprintf(stderr, "hash table is full: const_hashmap");
            abort();
        }
    }
    return index;
}

inline static bool const_hashmap_tryadd(const_hashmap_t *self, str id, i64 value) {
    u64 index = const_hashmap_find_index(self, id);
    if (entry_valid(&self->keys[index])) {
        return false;
    }
    self->keys[index] = id;
    self->values[index] = (const_entry_t){.value = value};
    return true;
}

inline static const_entry_t *const_hashmap_tryfind(const_hashmap_t *self, str id) {
    u64 index = const_hashmap_find_index(self, id);
    if (!entry_valid(&self->keys[index])) {
        return NULL;
    }
    return &self->values[index];
}

#undef entries
#undef array_len

#pragma once

#include "err.h"
#include "types.h"
#include "str.h"
#include "arr.h"

// hash entry

typedef struct {
    str key;
    reg_t value;
} hash_entry;

bool hash_entry_valid(const hash_entry *entry) {
    return entry->key.data;
}

void hash_entry_invalidate(hash_entry *entry) {
    entry->key.data = NULL;
}

// mini hashset

#define array_len ('Z' - 'A' + 1)

typedef hash_entry mini_hashset[array_len];

int hash(str id) {
    int index = id.data[0] - 'A';
    return index;
}

inline static hash_entry *find_entry(mini_hashset self, const str id) {
    printf("hash "), str_print(&id);
    int index = hash(id) % array_len;
    printf("hash was: %d\n", index);
    int start = index;

    while (hash_entry_valid(&self[index])) {
        if (str_eq(self[index].key, id)) {
            break;
        }
        index += 1;
        index %= array_len;
        if (index == start) {
            printf("hash tabel is full!");
            abort();
        }
    }
    return &self[index];
}

inline static bool has_entry(mini_hashset self, const str *s) {
    hash_entry *entry = find_entry(self, *s);
    return hash_entry_valid(entry);
}

inline static reg_t *overwrite_id(mini_hashset self, const token_t *id, const reg_t *value) {
    printf("ov\n");
    hash_entry *entry = find_entry(self, id->id);
    entry->key = id->id, entry->value = *value;
    return &entry->value;
}

inline static reg_t *add_id(mini_hashset self, str id, const reg_t *value) {
    printf("add\n");
    hash_entry *entry = find_entry(self, id);
    if (hash_entry_valid(entry)) {
        return NULL;
    } else {
        entry->key = id, entry->value = *value;
    }
    return &entry->value;
}

// arr mini hashset

#define MAX_DEPTH 5

typedef struct {
    mini_hashset data[MAX_DEPTH];
    mini_hashset *cur;
} arr_mini_hashset;

arr_mini_hashset local_ids;

static inline void arr_mini_hashset_init(arr_mini_hashset *arr) {
    arr->cur = arr->data;
}
static inline mini_hashset *arr_mini_hashset_push(arr_mini_hashset *arr) {
    if (arr->cur == arr->data + MAX_DEPTH) {
        fputs("arr was full", stderr);
        abort();
    }
    arr->cur++;
    printf("array is now %zd\n", arr->cur - arr->data);
    return arr->cur - 1;
}
static inline void arr_mini_hashset_pop(arr_mini_hashset *arr) {
    if (arr->cur == arr->data)
        return;
    for (int i = 0; i < array_len; ++i) {
        // arr->cur[0][i].key = str_null;
        hash_entry_invalidate(arr->cur[0] + i);
    }
    arr->cur--;
    printf("array is now %zd\n", arr->cur - arr->data);
}
static inline mini_hashset *arr_mini_hashset_top(arr_mini_hashset *arr) {
    if (arr->cur == arr->data)
        return 0;
    return arr->cur - 1;
}

inline static bool find_id(arr_mini_hashset *arr, const token_t *id, reg_t **out, int up) {
    printf("find\n");
    mini_hashset *target = (arr->cur - up);
    if (target < arr->data || target >= arr->data + MAX_DEPTH) {
        compile_err(id, "invalid access to local id scope\n");
        return false;
    }
    hash_entry *entry = find_entry(*target, id->id);
    *out = &entry->value;
    if (!hash_entry_valid(entry)) {
        return false;
    }
    return true;
}

#undef entries
#undef array_len

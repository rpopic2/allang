#pragma once

#include "err.h"
#include "types.h"
#include "str.h"
#include "arr.h"

typedef struct {
    str key;
    reg_t value;
} hash_entry;

const int array_len = 'Z' - 'A' + 1;

typedef hash_entry mini_hashset[array_len];

#define MAX_DEPTH 5

typedef struct {
    mini_hashset data[MAX_DEPTH];
    mini_hashset *cur;
} arr_mini_hashset;
static inline void arr_mini_hashset_new(arr_mini_hashset *arr) {
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
        arr->cur[0][i].key = str_null;
    }
    arr->cur--;
    printf("array is now %zd\n", arr->cur - arr->data);
}
static inline mini_hashset *arr_mini_hashset_top(arr_mini_hashset *arr) {
    if (arr->cur == arr->data)
        return 0;
    return arr->cur - 1;
}

arr_mini_hashset local_ids;

int hash(str id) {
    int index = id.data[0] - 'A';
    return index;
}

inline static hash_entry *find_entry(mini_hashset set, const token_t *id) {
    printf("hash "), str_print(&id->id);
    int index = hash(id->id) % array_len;
    printf("hash was: %d\n", index);
    int start = index;

    while (!str_empty(&set[index].key)) {
        if (str_eq(set[index].key, id->id)) {
            break;
        }
        index += 1;
        index %= array_len;
        if (index == start) {
            printf("hash tabel is full!");
            abort();
        }
    }
    return &set[index];
}

#define entries (*local_ids.cur)


inline static reg_t *overwrite_id(const token_t *id, const reg_t *value) {
    printf("ov\n");
    hash_entry *entry = find_entry(entries, id);
    entry->key = id->id, entry->value = *value;
    return &entry->value;
}

inline static reg_t *add_id(token_t *id, const reg_t *value) {
    printf("add\n");
    hash_entry *entry = find_entry(entries, id);
    if (!str_empty(&entry->key)) {
        compile_err(id, "duplicate identifier: "),
            str_print(&id->id);
    } else {
        entry->key = id->id, entry->value = *value;
    }
    return &entry->value;
}

inline static bool find_id(const token_t *id, reg_t **out, int up) {
    printf("find\n");
    mini_hashset *target = (local_ids.cur - up);
    if (target < local_ids.data || target >= local_ids.data + MAX_DEPTH) {
        compile_err(id, "invalid access to local id scope\n");
        abort();
    }
    hash_entry *entry = find_entry(*target, id);
    *out = &entry->value;
    if (str_empty(&entry->key)) {
        return false;
    }
    return true;
}

#undef entries

#pragma once

#include "str.h"
int hashmap_hash(str id) {
    int index = id.data[0];
    int end = id.end[-1];
    int len = (int)str_len(id);
    return index ^ end ^ len;
}

#define HASHMAP_GENERIC(T, array_len) \
 \
typedef struct { \
    str key; \
    T value; \
} hashentry_##T; \
 \
 \
typedef hashentry_##T hashmap_##T[array_len]; \
 \
bool hashentry_##T##_valid(const hashentry_##T *entry) { \
    return entry->key.data; \
} \
 \
inline static hashentry_##T *hashmap_##T##_find(hashmap_##T self, const str id) { \
    int index = hashmap_hash(id) % array_len; \
    str_printdnl(&id), printd(" -> hash was: %d\n", index); \
    int start = index; \
 \
    while (hashentry_##T##_valid(&self[index])) { \
        if (str_eq(self[index].key, id)) { \
            break; \
        } \
        printd("linear probe\n"); \
        index += 1; \
        index %= array_len; \
        if (index == start) { \
            fprintf(stderr, "hash tabel is full!"); \
            abort(); \
        } \
    } \
    return &self[index]; \
} \
 \
inline static hashentry_##T *hashmap_##T##_tryfind(hashmap_##T self, const str s) { \
    hashentry_##T *entry = hashmap_##T##_find(self, s); \
    if (hashentry_##T##_valid(entry)) \
        return entry; \
    else \
        return NULL; \
} \
 \
inline static T *hashmap_##T##_overwrite(hashmap_##T self, str id, const T *value) { \
    hashentry_##T *entry = hashmap_##T##_find(self, id); \
    entry->key = id, entry->value = *value; \
    return &entry->value; \
} \
 \
inline static T *hashmap_##T##_tryadd(hashmap_##T self, str id, const T *value) { \
    hashentry_##T *entry = hashmap_##T##_find(self, id); \
    if (hashentry_##T##_valid(entry)) { \
        return NULL; \
    } else { \
        entry->key = id, entry->value = *value; \
    } \
    return &entry->value; \
} \
 \


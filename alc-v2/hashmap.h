#pragma once

#define HASHMAP_GENERIC(T, array_len, hash_fn) \
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
    u64 real_hash = hash_fn(id); \
    u64 index = real_hash % array_len; \
    str_printdnl(&id), printd(" -> hash was: %"PRIu64"(from %"PRIu64")\n", index, real_hash); \
    u64 start = index; \
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
inline static T *hashmap_##T##_tryfind(hashmap_##T self, const str s) { \
    hashentry_##T *entry = hashmap_##T##_find(self, s); \
    if (hashentry_##T##_valid(entry)) \
        return &entry->value; \
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


#pragma once

#include "emit.h"

typedef struct {
    str key;
    symbol_t value;
} hashmap_entry;

#define array_len ('Z' - 'A' + 1)
typedef hashmap_entry hashmap[array_len];

hashmap fn_ids;

int hashmap_hash(str id) {
    int index = id.data[0] - 'A';
    return index;
}

bool hashmap_entry_valid(const hashmap_entry *entry) {
    return entry->key.data;
}

void hashmap_entry_invalidate(hashmap_entry *entry) {
    entry->key.data = NULL;
}

inline static hashmap_entry *hashmap_find(hashmap self, const str id) {
    printf("hashmap "), str_print(&id);
    int index = hashmap_hash(id) % array_len;
    printf("hash was: %d\n", index);
    int start = index;

    while (hashmap_entry_valid(&self[index])) {
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

inline static hashmap_entry *hashmap_tryfind(hashmap self, const str s) {
    hashmap_entry *entry = hashmap_find(self, s);
    if (hashmap_entry_valid(entry))
        return entry;
    else
        return NULL;
}

inline static symbol_t *hashmap_overwrite(hashmap self, str id, const symbol_t *value) {
    printf("ov\n");
    hashmap_entry *entry = hashmap_find(self, id);
    entry->key = id, entry->value = *value;
    return &entry->value;
}

inline static symbol_t *hashmap_tryadd(hashmap self, str id, const symbol_t *value) {
    printf("add\n");
    hashmap_entry *entry = hashmap_find(self, id);
    if (hashmap_entry_valid(entry)) {
        return NULL;
    } else {
        entry->key = id, entry->value = *value;
    }
    return &entry->value;
}

#undef array_len

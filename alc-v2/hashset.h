#include "err.h"
#include "types.h"
#include "str.h"

typedef struct {
    str key;
    reg_t value;
} hash_entry;

const int array_len = 'Z' - 'A' + 1;
hash_entry entries[array_len];

int hash(str id) {
    int index = id.data[0] - 'A';
    return index;
}

inline static hash_entry *find_entry(const token_t *id) {
    printf("hash "), str_print(&id->id);
    int index = hash(id->id) % array_len;
    printf("hash was: %d\n", index);
    int start = index;

    while (!str_empty(&entries[index].key)) {
        if (str_eq(entries[index].key, id->id)) {
            break;
        }
        index += 1;
        index %= array_len;
        if (index == start) {
            printf("hash tabel is full!");
            abort();
        }
    }
    return &entries[index];
}

inline static reg_t *overwrite_id(const token_t *id, const reg_t *value) {
    printf("ov\n");
    hash_entry *entry = find_entry(id);
    entry->key = id->id, entry->value = *value;
    return &entry->value;
}

inline static reg_t *add_id(token_t *id, const reg_t *value) {
    printf("add\n");
    hash_entry *entry = find_entry(id);
    if (!str_empty(&entry->key)) {
        compile_err(id, "duplicate identifier: "),
            str_print(&id->id);
    } else {
        entry->key = id->id, entry->value = *value;
    }
    return &entry->value;
}

inline static bool find_id(const token_t *id, reg_t **out) {
    printf("find\n");
    hash_entry *entry = find_entry(id);
    *out = &entry->value;
    if (str_empty(&entry->key)) {
        return false;
    }
    return true;
}


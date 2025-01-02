#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

#define BUCKET_SIZE 983

void hashmap_new(struct HashMap *map) {
    map->bucket = calloc(BUCKET_SIZE, sizeof(char));
}

void hashmap_free(struct HashMap *map) {
    free(map->bucket);
}

size_t hash_fn(const char *str) {
    unsigned long len = strlen(str);
    size_t hash = 0;
    int counter = 0;
    const char *p = str;
    for (; *p != '\0'; ++p) {
        printf("%c", *p);
        hash += *p;
        if (++counter > 3)
            break;
    }
    hash -= 0x40;
    printf("\n");
    if (len <= 4)
        return hash;

    counter = 0;
    for (const char *q = str + len; q != p; --q) {
        printf("%c", *q);
        hash += *q;
        if (++counter > 4)
            break;
    }
    printf("\n");
    return hash;
}

void hashmap_add(struct HashMap *map, const char *str, size_t num) {
    size_t hash = hash_fn(str);
    if (map->bucket[hash]) {
        printf("collision happened: %s\n", str);
        return;
    }
    map->bucket[hash] = num;
    printf("map %s: %zu -> %zu\n", str, num, hash);
}

size_t hashmap_get(struct HashMap *map, const char *str) {
    size_t hash = hash_fn(str);
    return map->bucket[hash];
}

/* int main() {
    struct HashMap map;
    hashmap_new(&map);

    hashmap_add(&map, "count", 30);
    hashmap_add(&map, "allocator_size_ptr", 30);
    hashmap_add(&map, "allocator_size_ptrptr", 30);
    hashmap_add(&map, "allocator_size_ptrptr2", 30);
    hashmap_add(&map, "get_boot_config_from_initrd", 30);
    hashmap_add(&map, "parse_early_param", 30);
    hashmap_add(&map, "parse_early_options", 30);
    hashmap_add(&map, "xbc_node_for_each_key_value", 30);
    hashmap_add(&map, "A", 20);
    hashmap_add(&map, "B", 20);
    hashmap_add(&map, "A0", 20);
    hashmap_add(&map, "AA", 20);
    hashmap_add(&map, "AB", 21);
    hashmap_add(&map, "BA", 21);
    hashmap_add(&map, "C", 21);
    hashmap_add(&map, "i", 21);
    hashmap_add(&map, "j", 25);
    hashmap_add(&map, "len", 3);
    hashmap_add(&map, "length", 3);
    hashmap_add(&map, "lenggaanel", 3);
    hashmap_add(&map, "lenganel", 3);

    hashmap_free(&map);
}
*/


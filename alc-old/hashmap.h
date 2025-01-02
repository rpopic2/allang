#pragma once
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct HashMap {
    char *bucket;
};

void hashmap_new(struct HashMap *map);
void hashmap_free(struct HashMap *map);
void hashmap_add(struct HashMap *map, const char *str, size_t num);
size_t hashmap_get(struct HashMap *map, const char *str);

#include "types.h"
#include "str.h"

const size_t array_len = 'Z' - 'A' + 1;
reg_t entries[array_len];

reg_t *add_id(str id, register_dst type, int offset) {
    int index = id.data[0] - 'A';
    if (index < 0 || index >= (int)array_len) {
        fprintf(stderr, "array access out of bounds");
        abort();
    }
    entries[index] = (reg_t) {type, offset};
    return &entries[index];
}

reg_t *find_id(const str *id) {
    return &entries[(int)id->data[0]];
}


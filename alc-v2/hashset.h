#include "types.h"
#include "str.h"

reg_t entries['z' - 'a' + 1];

reg_t *add_id(str id, register_dst type, int offset) {
    int index = id.data[0];
    entries[index] = (reg_t) {type, offset};
    return &entries[index];
}

reg_t *find_id(const str *id) {
    return &entries[(int)id->data[0]];
}


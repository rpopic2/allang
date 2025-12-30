#include "types.h"
#include "str.h"

reg_t entries['z' - 'a' + 1];

void add_id(str id, register_dst type, int offset) {
    entries[(int)id.data[0]] = (reg_t) {type, offset};
}

reg_t *find_id(const str *id) {
    return &entries[(int)id->data[0]];
}


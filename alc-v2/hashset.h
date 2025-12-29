#include "types.h"
#include "str.h"

entry entries['z' - 'a' + 1];

void add_id(str id, register_dst type, int offset) {
    entries[(int)id.data[0]] = (entry) {type, offset};
}

entry *find_id(const str *id) {
    return &entries[(int)id->data[0]];
}


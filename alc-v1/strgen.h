#include "error.h"
#include "slice.h"
#include <stdlib.h>

char buffer[0x4];
int _strgen_index = 0;

str strgen_next() {
    buffer[0] = _strgen_index + '0';
    if (_strgen_index >= 100) {
        CompileErr("Internal Error: strgen max exceeds");
        exit(2);
    }
    if (_strgen_index >= 10) {
        buffer[1] = _strgen_index / 10;
    }
    _strgen_index += 1;
    return (str) { buffer, strlen(buffer) };
}


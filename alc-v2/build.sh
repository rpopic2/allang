if [[ "$(uname -s)" != MINGW* ]]; then
    EXTRA_FLAGS+=" -fsanitize=undefined"
fi

clang -DDEBUG_TIMER -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -O -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Wno-unused-function -g -std=c11 $EXTRA_FLAGS main.c "$@" -o alc

if [[ "$(uname -s)" != MINGW* ]]; then
    if clang -fsanitize=undefined -x c /dev/null -o /dev/null 2>/dev/null; then
        EXTRA_FLAGS+=" -fsanitize=undefined"
    else
        EXTRA_FLAGS+=" -fsanitize=undefined -fno-sanitize-link-runtime -lubsan"
    fi
fi
if [ "$(uname -o)" = Android ]; then
    EXTRA_FLAGS+=" -lexecinfo -rdynamic -fno-omit-frame-pointer"
fi

echo $EXTRA_FLAGS

clang -DDEBUG_TIMER -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Wno-unused-function -g -std=c11 $EXTRA_FLAGS main.c "$@" -o alc

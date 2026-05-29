if [ "$(uname -o)" = Android ]; then
    EXTRA_FLAGS+=" -lexecinfo -rdynamic -fno-omit-frame-pointer"
fi

echo $EXTRA_FLAGS

clang -DDEBUG_TIMER -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Wno-unused-function -g -std=c11 $EXTRA_FLAGS main.c "$@" -o alc

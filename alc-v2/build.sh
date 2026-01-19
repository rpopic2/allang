clang -DDEBUG_TIMER -O -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Wno-unused-function -g -std=c11 -fsanitize=undefined main.c "$@" -o alc

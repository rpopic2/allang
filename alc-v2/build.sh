clang -DDEBUG_TIMER -D_CRT_SECURE_NO_WARNINGS -O -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Wno-unused-function -g -std=c11 main.c "$@" -o alc

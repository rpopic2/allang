clang -O -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Wno-unused-function -std=c11 -fsanitize=undefined main.c $1 $2 -o alc

#!/bin/bash

clang $2 hashmap.c main.c &&  \
    ./a.out $1



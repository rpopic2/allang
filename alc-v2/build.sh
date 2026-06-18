#!/usr/bin/env bash

if [ $# -eq 0 ]; then
    case "$(uname -s):$(uname -m)" in
        Linux:x86_64)
            EMIT_ARCH="bin-x86_64.c"
            EMIT_OS="exe-elf-x86_64.c"
            ;;
        Linux:aarch64|Linux:arm64)
            EMIT_ARCH="bin-aarch64.c"
            EMIT_OS="exe-elf-aarch64.c"
            ;;
        Darwin:arm64|Darwin:aarch64)
            EMIT_ARCH="bin-aarch64.c"
            EMIT_OS="exe-macho.c"
            ;;
        MINGW*:x86_64|MSYS*:x86_64|CYGWIN*:x86_64)
            EMIT_ARCH="bin-x86_64.c"
            EMIT_OS="exe-pe-x86_64.c"
            ;;
        *)
            echo "FATAL: binary backend not supported on $(uname -s)/$(uname -m)" >&2
            exit 1
            ;;
    esac
fi

echo $EMIT_ARCH $EMIT_OS $EXTRA_FLAGS

clang -DDEBUG_TIMER -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Wno-unused-function -g -std=c11 $EXTRA_FLAGS main.c diagnostics.c $EMIT_ARCH $EMIT_OS "$@" -o alc


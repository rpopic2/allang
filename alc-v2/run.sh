#!/usr/bin/env bash

if [ $# -ne 1 ]; then
    echo "usage: run.sh <source.al>" >&2
    exit 1
fi

SOURCE="$1"

case "$(uname -s):$(uname -m)" in
    Linux:x86_64)
        EMIT_ARCH="emit-x86_64-bin.c"
        EMIT_EXE="emit-elf-x86_64-exe.c"
        ;;
    Linux:aarch64|Linux:arm64)
        EMIT_ARCH="emit-aarch64-bin.c"
        EMIT_EXE="emit-elf-aarch64-exe.c"
        ;;
    Darwin:arm64|Darwin:aarch64)
        EMIT_ARCH="emit-aarch64-bin.c"
        EMIT_EXE="emit-macho-aarch64-exe.c"
        ;;
    *)
        echo "error: binary backend not supported on $(uname -s)/$(uname -m)" >&2
        exit 1
        ;;
esac

UBSAN_OPTIONS=print_stacktrace=1

./build.sh "$EMIT_ARCH" "$EMIT_EXE" || exit $?

FILENAME="${SOURCE%.*}"
./alc "$SOURCE" && ./"$FILENAME"

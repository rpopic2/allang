#!/usr/bin/env bash

if [ $# -ne 1 ]; then
    echo "usage: run.sh <source.al>" >&2
    exit 1
fi

SOURCE="$1"

EXT=""
case "$(uname -s):$(uname -m)" in
    Linux:x86_64)
        EMIT_ARCH="exe-x86_64.c"
        EMIT_OS="exe-elf-x86_64.c"
        ;;
    Linux:aarch64|Linux:arm64)
        EMIT_ARCH="exe-aarch64.c"
        EMIT_OS="exe-elf-aarch64.c"
        ;;
    Darwin:arm64|Darwin:aarch64)
        EMIT_ARCH="exe-aarch64.c"
        EMIT_OS="exe-macho.c"
        ;;
    MINGW*:x86_64|MSYS*:x86_64|CYGWIN*:x86_64)
        EMIT_ARCH="exe-x86_64.c"
        EMIT_OS="exe-pe-x86_64.c"
        EXT=".exe"
        ;;
    *)
        echo "FATAL: binary backend not supported on $(uname -s)/$(uname -m)" >&2
        exit 1
        ;;
esac

UBSAN_OPTIONS=print_stacktrace=1

./build.sh "$EMIT_ARCH" "$EMIT_OS" || exit $?

FILENAME="${SOURCE%.*}"
./alc "$SOURCE" || exit $?

BINARY="./$FILENAME$EXT"
case "$(uname -s)" in
    Darwin)
        otool -tvV "$BINARY" > "$FILENAME.s"
        ;;
    *)
        objdump -d "$BINARY" > "$FILENAME.s"
        ;;
esac

"$BINARY"

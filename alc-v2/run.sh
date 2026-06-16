#!/usr/bin/env bash

if [ $# -ne 1 ]; then
    echo "usage: run.sh <source.al>" >&2
    exit 1
fi

SOURCE="$1"


case "$(uname -s)" in
    Linux)
        EMIT_OS="emit-linux.c"
        ;;
    Darwin)
        EMIT_OS="emit-macos.c"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        EMIT_OS="emit-windows.c"
        ;;
    *)
        echo "Unknown OS: $(uname -s)" >&2
        exit 1
        ;;
esac

case "$(uname -m)" in
    x86_64)
        EMIT_ARCH="emit-x86_64.c"
        ;;
    aarch64|arm64)
        EMIT_ARCH="emit-aarch64.c"
        ;;
    *)
        echo "Unknown architecture: $(uname -m)" >&2
        exit 1
        ;;
esac

UBSAN_OPTIONS=print_stacktrace=1

./build.sh "$EMIT_ARCH" "$EMIT_OS" || exit $?

FILENAME="${SOURCE%.*}"
./alc "$SOURCE" && ./"$FILENAME"

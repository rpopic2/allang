#!/usr/bin/env bash
if [ "$(uname -o)" = Android ]; then
    EXTRA_FLAGS+=" -lexecinfo -rdynamic -fno-omit-frame-pointer -fsanitize=undefined"
elif [[ "$(uname -s)" = Linux ]]; then
    EXTRA_FLAGS+=" -fsanitize=undefined -fno-sanitize-link-runtime -lubsan -rdynamic"
elif [[ "$(uname -s)" == MINGW* || "$(uname -s)" == MSYS* || "$(uname -s)" == CYGWIN* ]]; then
    # Windows clang's ubsan runtime needs the clang-cl driver to link; use
    # trap mode so UB is still caught (crashes at the fault site) with no runtime.
    EXTRA_FLAGS+=" -fsanitize=undefined -fsanitize-trap=undefined"
else
    EXTRA_FLAGS+=" -fsanitize=undefined -lubsan"
fi

# Auto-detect emit files if first argument looks like a .al file or is missing
if [ $# -eq 0 ] || [[ "$1" == *.al ]]; then
    case "$(uname -s)" in
        Linux)
            EMIT_OS="asm-linux.c"
            ;;
        Darwin)
            EMIT_OS="asm-macos.c"
            ;;
        MINGW*|MSYS*|CYGWIN*)
            EMIT_OS="asm-windows.c"
            ;;
        *)
            echo "Unknown OS: $(uname -s)" >&2
            exit 1
            ;;
    esac

    case "$(uname -m)" in
        x86_64)
            EMIT_ARCH="asm-x86_64.c"
            ;;
        aarch64|arm64)
            EMIT_ARCH="asm-aarch64.c"
            ;;
        *)
            echo "Unknown architecture: $(uname -m)" >&2
            exit 1
            ;;
    esac

    set -- "$EMIT_ARCH" "$EMIT_OS" "$@"
fi

echo $EXTRA_FLAGS

clang -DDEBUG_TIMER -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Wno-unused-function -g -std=c11 $EXTRA_FLAGS main.c diagnostics.c "$@" -o alc

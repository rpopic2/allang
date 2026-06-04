if [[ "$(uname -s)" = Linux ]]; then
    EXTRA_FLAGS+=" -fsanitize=undefined -fno-sanitize-link-runtime -lubsan"
elif [[ "$(uname -s)" != MINGW* ]]; then
    EXTRA_FLAGS+=" -fsanitize=undefined"
fi
if [ "$(uname -o)" = Android ]; then
    EXTRA_FLAGS+=" -lexecinfo -rdynamic -fno-omit-frame-pointer"
fi

# Auto-detect emit files if first argument looks like a .al file or is missing
if [ $# -eq 0 ] || [[ "$1" == *.al ]]; then
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

    set -- "$EMIT_ARCH" "$EMIT_OS" "$@"
fi

echo $EXTRA_FLAGS

clang -DDEBUG_TIMER -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Wno-unused-function -g -std=c11 $EXTRA_FLAGS main.c diagnostics.c "$@" -o alc

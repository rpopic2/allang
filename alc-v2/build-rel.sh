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

clang -DNDEBUG -O2 -std=c11 main.c diagnostics.c "$EMIT_ARCH" "$EMIT_OS" "$@" -o alc

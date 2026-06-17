#!/usr/bin/env bash

# test-all.sh equivalent for the binary backend.
#
# Unlike test-all.sh, the binary backend makes alc emit a complete, directly
# runnable executable (output_ext is ""), so there is no separate clang step:
# alc produces tests/<name>, which is run directly.

cd "$(dirname "$0")"

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


echo "==> Building alc ($EMIT_ARCH $EMIT_OS)..."
if ! ./build.sh "$EMIT_ARCH" "$EMIT_OS"; then
    echo "FATAL: build failed"
    exit 1
fi
echo ""

PASS=0
FAIL=0
FAIL_NAMES=()

for AL_FILE in tests/*.al; do
    NAME="${AL_FILE%.*}"
    printf "  %-30s" "$AL_FILE"

    ALC_OUT=$(./alc "$AL_FILE" 2>&1)
    ALC_EXIT=$?
    if [ $ALC_EXIT -ne 0 ]; then
        printf "FAIL (alc)\n"
        ((FAIL++))
        FAIL_NAMES+=("$AL_FILE (alc: $(echo "$ALC_OUT" | grep "error"))")
        continue
    fi

    if [ ! -f "$NAME$EXT" ]; then
        printf "FAIL (no executable)\n"
        ((FAIL++))
        FAIL_NAMES+=("$AL_FILE (no executable emitted)")
        continue
    fi

    "./$NAME$EXT" >/dev/null 2>&1
    RUN_EXIT=$?
    if [ $RUN_EXIT -ne 0 ]; then
        printf "FAIL (exit %d)\n" $RUN_EXIT
        ((FAIL++))
        FAIL_NAMES+=("$AL_FILE (exit $RUN_EXIT)")
    else
        printf "PASS\n"
        ((PASS++))
    fi
done

echo ""
echo "Results: $PASS passed, $FAIL failed"

if [ ${#FAIL_NAMES[@]} -gt 0 ]; then
    echo "Failed:"
    for F in "${FAIL_NAMES[@]}"; do
        echo "  - $F"
    done
    exit 1
fi

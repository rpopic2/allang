#!/usr/bin/env bash

cd "$(dirname "$0")"

EXTRA_CLANG_FLAGS=""
if [[ "$(uname -m)" == x86* ]]; then
    EXTRA_CLANG_FLAGS="-masm=intel"
fi

echo "==> Building alc..."
if ! ./build.sh; then
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
        FAIL_NAMES+=("$AL_FILE (alc: $(echo "$ALC_OUT" | tail -1))")
        continue
    fi

    if [ ! -f "${NAME}.s" ]; then
        printf "PASS (syntax only)\n"
        ((PASS++))
        continue
    fi

    CLANG_OUT=$(clang "${NAME}.s" $EXTRA_CLANG_FLAGS -o "${NAME}_bin" 2>&1)
    CLANG_EXIT=$?
    if [ $CLANG_EXIT -ne 0 ]; then
        printf "FAIL (clang)\n"
        ((FAIL++))
        FAIL_NAMES+=("$AL_FILE (clang: $(echo "$CLANG_OUT" | tail -3))")
        continue
    fi

    "./${NAME}_bin"
    RUN_EXIT=$?
    rm -f "${NAME}_bin"
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

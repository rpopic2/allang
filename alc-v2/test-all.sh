#!/usr/bin/env bash

now_ms() {
    local t="${EPOCHREALTIME//[.,]/}"
    echo $(( 10#$t / 1000 ))
}

ALL_START=$(now_ms)

cd "$(dirname "$0")"

EXTRA_CLANG_FLAGS=""
if [[ "$(uname -m)" == x86* ]]; then
    EXTRA_CLANG_FLAGS="-masm=intel"
fi

echo "==> Building alc..."
if ! ./build-rel.sh; then
    echo "FATAL: build failed"
    exit 1
fi
BUILD_MS=$(( $(now_ms) - ALL_START ))
echo "$BUILD_MS ms"
echo ""

PASS=0
FAIL=0
TOTAL_MS=0
FAIL_NAMES=()
declare -A STATUS

echo "==> Compiling (alc)..."
for AL_FILE in tests/*.al; do
    NAME="${AL_FILE%.*}"
    printf "  %-30s" "$AL_FILE"

    T_START=$(now_ms)
    ALC_OUT=$(./alc "$AL_FILE" 2>&1)
    ALC_EXIT=$?
    T_MS=$(( $(now_ms) - T_START ))
    ((TOTAL_MS += T_MS))
    printf "%6d ms  " "$T_MS"
    if [ $ALC_EXIT -ne 0 ]; then
        printf "FAIL (alc)\n"
        STATUS[$AL_FILE]=fail
        ((FAIL++))
        FAIL_NAMES+=("$AL_FILE (alc: $(echo "$ALC_OUT" | grep "error"))")
    elif [ ! -f "${NAME}.s" ]; then
        printf "PASS (syntax only)\n"
        STATUS[$AL_FILE]=syntax
        ((PASS++))
    else
        printf "ok\n"
        STATUS[$AL_FILE]=ok
    fi
done
echo ${TOTAL_MS} ms total

echo ""
echo "==> Assembling (clang)..."
CLANG_START=$(now_ms)
JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
CLANG_TMP=$(mktemp -d)
for AL_FILE in tests/*.al; do
    [ "${STATUS[$AL_FILE]}" = ok ] || continue
    NAME="${AL_FILE%.*}"

    while [ "$(jobs -rp | wc -l)" -ge "$JOBS" ]; do wait -n; done
    {
        OUT=$(clang "${NAME}.s" $EXTRA_CLANG_FLAGS -o "${NAME}_bin" 2>&1)
        echo "$?" > "$CLANG_TMP/$(basename "$NAME").exit"
        printf '%s' "$OUT" > "$CLANG_TMP/$(basename "$NAME").out"
    } &
done
wait

for AL_FILE in tests/*.al; do
    [ "${STATUS[$AL_FILE]}" = ok ] || continue
    NAME="${AL_FILE%.*}"
    B=$(basename "$NAME")

    if [ "$(cat "$CLANG_TMP/$B.exit")" -ne 0 ]; then
        printf "FAIL (clang)\n"
        STATUS[$AL_FILE]=fail
        ((FAIL++))
        FAIL_NAMES+=("$AL_FILE (clang: $(tail -3 "$CLANG_TMP/$B.out"))")
    fi
done
CLANG_MS=$(( $(now_ms) - CLANG_START ))
echo "  clang total: ${CLANG_MS} ms"

echo ""
echo "==> Running..."
RUN_START=$(now_ms)
for AL_FILE in tests/*.al; do
    [ "${STATUS[$AL_FILE]}" = ok ] || continue
    NAME="${AL_FILE%.*}"

    RUN_OUT=$("./${NAME}_bin" 2>&1)
    RUN_EXIT=$?
    if [ $RUN_EXIT -ne 0 ]; then
        ((FAIL++))
        FAIL_NAMES+=("$AL_FILE (exit $RUN_EXIT)")
    else
        ((PASS++))
    fi
done
RUN_MS=$(( $(now_ms) - RUN_START ))
echo "  run total: ${RUN_MS} ms"

echo ""
ALL_MS=$(( $(now_ms) - ALL_START ))
echo "Results: $PASS passed, $FAIL failed, $ALL_MS ms"

if [ ${#FAIL_NAMES[@]} -gt 0 ]; then
    echo "Failed:"
    for F in "${FAIL_NAMES[@]}"; do
        echo "  - $F"
    done
    exit 1
fi

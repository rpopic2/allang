
if [ $# -eq 0 ]; then
    echo "usage: run-asm.sh [<asm-arch.c> <asm-os.c>] <source.al>" >&2
    exit 1
fi

if [[ "$(uname -m)" == x86* ]]; then
    EXTRA_FLAGS+=" -masm=intel"
fi

UBSAN_OPTIONS=print_stacktrace=1

if [[ "$1" == *.al ]]; then
    SOURCE="$1"
    ./build-asm.sh "${@:2}"
    BUILD_EXIT=$?
else
    SOURCE="$3"
    ./build-asm.sh $1 $2 "${@:4}"
    BUILD_EXIT=$?
fi

[ $BUILD_EXIT -ne 0 ] && exit $BUILD_EXIT

FILENAME=$(echo "${SOURCE%.*}")
$(which time) ./alc "$SOURCE" \
    && clang "$FILENAME".s $EXTRA_FLAGS -o $FILENAME \
    && ./$FILENAME

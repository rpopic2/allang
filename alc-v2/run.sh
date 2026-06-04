
if [[ "$(uname -m)" == x86* ]]; then
    EXTRA_FLAGS+=" -masm=intel"
fi

UBSAN_OPTIONS=print_stacktrace=1

# If first argument is a .al file (no emit files provided), let build.sh auto-detect
if [[ "$1" == *.al ]]; then
    SOURCE="$1"
    ./build.sh "${@:2}"
    BUILD_EXIT=$?
else
    # Normal case with explicit emit files: $1=$emit-arch, $2=$emit-os, $3=$source
    SOURCE="$3"
    ./build.sh $1 $2 "${@:4}"
    BUILD_EXIT=$?
fi

[ $BUILD_EXIT -ne 0 ] && exit $BUILD_EXIT

FILENAME=$(echo "${SOURCE%.*}")
$(which time) ./alc "$SOURCE" \
    && clang "$FILENAME".s $EXTRA_FLAGS -o $FILENAME \
    && ./$FILENAME

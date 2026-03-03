
if [[ "$(uname -m)" == x86* ]]; then
    EXTRA_FLAGS+=" -masm=intel"
fi

UBSAN_OPTIONS=print_stacktrace=1
FILENAME=$(echo "${3%.*}")
./build.sh $1 $2 "${@:4}" \
    && $(which time) ./alc $3 \
    && clang "$FILENAME".s $EXTRA_FLAGS -o $FILENAME \
    && ./$FILENAME

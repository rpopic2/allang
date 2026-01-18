UBSAN_OPTIONS=print_stacktrace=1
FILENAME=$(echo "${3%.*}")
./build.sh $1 $2 \
    && time ./alc $3 \
    && clang $FILENAME.s -o $FILENAME

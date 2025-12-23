FILENAME=$(echo "${1%.*}")
./build.sh \
    && time ./alc $1 \
    && clang $FILENAME.s -o $FILENAME

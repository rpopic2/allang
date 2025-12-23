FILENAME=$(echo "${1%.*}")
./build.sh \
    && time ./alc $1 \
    && time clang $FILENAME.s -o $FILENAME

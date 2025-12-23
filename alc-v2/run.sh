FILENAME=$(echo "${2%.*}")
./build.sh $1 \
    && time ./alc $2 \
    && clang $FILENAME.s -o $FILENAME

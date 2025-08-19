clang ./main.c -O -g -o alc \
    && ./alc $1 \
    && ld main.o -lSystem -syslibroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.2.sdk -o main \
    && otool -tvj main \
    && ./main $2 $3 $4 \
echo $?

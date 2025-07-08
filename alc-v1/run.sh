clang ./main.c -O -g \
    && ./a.out \
    && ld main.o -lSystem -syslibroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.2.sdk -o main \
    && otool -tvj main \
    && ./main \
    && echo $?
echo $?

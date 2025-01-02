clang ./macho-exe.c -o macho-exe && ./macho-exe && hexdump -C ./machoexe.out
chmod 755 ./machoexe.out
./machoexe.out


clang ./macho-reloc.c -o macho-reloc && ./macho-reloc && hexdump -C ./machoreloc.out && clang machoreloc.out -o machoreloc && ./machoreloc && echo $?

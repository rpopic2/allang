clang ./macho-reloc.c -o macho-reloc && ./macho-reloc && hexdump -C ./machoreloc.out && ld machoreloc.out -o machoreloc && ./machoreloc && echo $?

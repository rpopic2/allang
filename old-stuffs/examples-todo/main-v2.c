// same programme in c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *FILE_NAME = "todo.txt";

int filelen(FILE *file) {
    fseek(file, 0L, SEEK_END);
    long len = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return len;
}

void usage(void) {
    printf("\
usage\n\
create entry test: -c test\n\
delete entry at 0: -d 0\n\
");
}

// out params because c does not support multiple return values
void read_file(FILE **file, char **buf, int *file_len) {
    *file = fopen(FILE_NAME, "r");
    *buf = malloc(1024);
    *file_len = filelen(*file);
    fread(*buf, sizeof (char), *file_len, *file);
}

void read_file2(FILE *file, char *buf, int file_len) {
    file = fopen(FILE_NAME, "r");
    buf = malloc(1024);
    file_len = filelen(file);
    fread(buf, sizeof (char), file_len, file);
}

char *find_line_end(char *ptr) {
    while (1) {
        ptr++;
        if (*ptr == '\n' || *ptr == '\0') {
            return ptr + 1;
        }
    }
    return ptr;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        FILE *file = fopen(FILE_NAME, "r");
        char *buf = malloc(1024);
        int len = filelen(file);
        printf("%s", buf);
        exit(0);
    }

    char *arg1 = argv[1];
    if (arg1[0] != '-') {
        usage();
        exit(0);
    }

    char cmd = arg1[1];
    if (cmd == 'c') {
        FILE *file = fopen(FILE_NAME, "a");

        char *item = argv[2];
        int item_len = strlen(item);
        fwrite(item, sizeof (char), item_len, file);
        fwrite("\n", sizeof (char), 1, file);
        fclose(file);
        exit(0);
    }

    if (cmd == 'd') {
        FILE *file;
        char *buf;
        int filelen;
        read_file(&file, &buf, &filelen);
        file = fopen(FILE_NAME, "w");

        char *ptr = buf;
        char *idxs = argv[2];
        int idx = atoi(idxs);

        for (int i = 0; i < idx; ++i) {
            ptr = find_line_end(ptr);
        }
        fwrite(buf, sizeof (char), ptr - buf, file);
        ptr = find_line_end(ptr);
        fwrite(ptr, sizeof (char), filelen - (ptr - buf), file);
        fclose(file);
        exit(0);
    }
    usage();
    exit(1);
}

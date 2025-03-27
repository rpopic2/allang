#include <stdio.h>

int main(void) {
    int a, b;
    scanf("%d %d", &a, &b);
    char c;
    if (a < b) {
        c = '<';
    } else if (a > b) {
        c = '>';
    } else {
        c = '=';
    }
    printf("%c", c);
}


#include <stdio.h>

void putint(int n);

int main(int argc, char const *argv[]) {
    for (int i = 0; i < 10; ++i) {
        putint(i);
    }
    putchar('\n');

    for (int i = 199; i < 1000000; i *= 12) {
        putint(i);
        printf(", printf=%d\n", i);
    }

    return 0;
}

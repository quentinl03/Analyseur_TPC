#include <stdio.h>
#include <assert.h>


int putchar(int c);

int main(void) {
    char* str = "Hello World!\n";
    char* cpy = str;

    for (; *cpy; cpy++) {
        int c = putchar(*cpy);
        assert(c == *cpy);
    }

    printf("%s", str);
}

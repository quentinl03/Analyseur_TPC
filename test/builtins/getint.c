#include <stdio.h>

int getint(void);

int main(int argc, char const *argv[]) {
    int n;
    while (1) {
        printf("Enter a number:\n");
        n = getint();
        printf("You entered %d\n\n", n);
    }

    return 0;
}

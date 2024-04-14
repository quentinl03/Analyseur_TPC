#include <stdio.h>

int getchar(void);

int main(int argc, char const *argv[])
{
    int c;
    while ((c = getchar()) != EOF) {
        printf("%d\n", c);
    }

    return 0;
}

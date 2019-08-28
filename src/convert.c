#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    unsigned char a = atoi(argv[1]);
    char b = a;

    printf("input: %u, output: %d\n", a, b);
    printf("input: 0x%02X, output: 0x%02X\n", a, b);
}

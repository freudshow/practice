#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long u64;

u64 fibRec(u64 n)
{
    u64 fib;
    if (n > 1) {
        fib = fibRec(n -1) + fibRec(n - 2);
    } else {
        fib = n;
    }

    return fib;
}

u64 fibLinear(u64 n)
{
    u64 f = 0;
    u64 g = 1;

    if (n > 1) {
        n -= 1;
        while (n--) {
            g = g+f;
            f = g-f;
        }
    } else {
        g = n;
    }

    return g;
}

u64 main(u64 argc, char **argv)
{
    unsigned int n = 0;

    for (n = 0; n <= atol(argv[1]); n++)
        printf("fibonnaci(%u) = %ll \n", n, fibLinear(n));

    return 0;
}

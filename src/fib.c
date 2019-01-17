#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;
typedef unsigned long long u64;

u64 fibRec(u32 n)
{
    u64 fib;
    if (n > 1) {
        fib = fibRec(n -1) + fibRec(n - 2);
    } else {
        fib = n;
    }

    return fib;
}

u64 fibLinear(u32 n)
{
    u64 f = 0;
    u64 g = 1;

    if (n > 1) {
        while (--n) {
            g = g+f; // g <- a[n+1] <- (a[n] + a[n-1])
            f = g-f; // f <- (a[n+1] - a[n-1] = a[n])
        }
    } else {
        g = n;
    }

    return g;
}

int main(int argc, char **argv)
{
	u32 n = 0;

    for (n = 0; n <= atol(argv[1]); n++)
        printf("fibonnaci(%u) = %llu\n", n, fibLinear(n));

    return 0;
}

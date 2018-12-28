#include <stdlib.h>
#include <time.h>

int main(void)
{
    int i = 0;
    srandom((int) time(NULL));
    
    for (i=0; i<10; i++) {
        printf("%d\n", random());
    }
    
    exit(0);
}
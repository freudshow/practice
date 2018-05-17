#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>



int main(void)
{
    printf("sizeof(long int): %d\n, sizeof (fd_set): %d\n, sizeof (__fd_mask): %d\n, __FD_SETSIZE: %d\n",
            sizeof(long int), sizeof (fd_set), sizeof (__fd_mask), __FD_SETSIZE);
    exit(0);
}

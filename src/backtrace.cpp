#include <iostream> 
#include <execinfo.h>
#include <signal.h>

using namespace std;  
 
 
 void test_sig(int sig)
 {
    void *array[10];
    int size, i;
    char **strings;
 
    fprintf(stderr, "\nSegmentation fault\n");
    size = backtrace(array, 10);
    fprintf(stderr, "\nBacktrace (%d deep):\n", size);
 
    strings = backtrace_symbols(array, size);
    for (i=0; i < size; i++)
    {
        fprintf(stderr, "%d: %s, signal is %d\n", i, strings[i], sig);
    }
 
    free(strings);
    strings = NULL;
 }
 
int main() 
{ 
    signal(SIGSEGV, test_sig);
    int *p = nullptr;
    *p = 1;
}

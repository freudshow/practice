#include <stdio.h>
#include <stdlib.h>

#define MAX(x, y)   ({\
                        typeof(x) _x = x;\
                        typeof(x) _y = y;\
                        (void)(&_x == &_y);\
                        _x > _y ? _x : _y;\
                    })

float foo(float a)
{
    return a;
}
                        
int main(void)
{
    int a = 5, b = 8;
    
    printf("max of a, b: %d\n", MAX(a,b));
    
    typeof(foo(4.5)) c = 8.9;
    printf("c: %f\n", c);
    return 0;
}
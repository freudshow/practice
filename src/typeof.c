#include <stdio.h>
#include <stdlib.h>

#define MAX(x, y)   ({\
                        typeof(x) _x = x;\
                        typeof(y) _y = y;\
                        _x > _y ? _x : _y;\
                    })

#define auto_max(x,y) ({\
                        __auto_type _x = x;\
                        __auto_type _y = y;\
                        _x > _y ? _x : _y;\
                    })

float foo(float a)
{
    return a;
}
                        
int main(void)
{
    int a = 5, b = 8;
    float c = 9.4;
    
    printf("max of a, b: %d\n", MAX(a,b));
    printf("max of a, c: %d\n", MAX(a,c));
    printf("max of a, c: %d\n", auto_max(a,c));
    typeof(foo(4.5)) d = 8.9;
    printf("d: %f\n", d);
    return 0;
}
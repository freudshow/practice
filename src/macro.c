#include <stdio.h>
#include <stdlib.h>

#define mma(x)  #x
#define mmb(x)  fuck_##x

int main(int argc, char const *argv[]) {
  char fuck_you[] = "fuck you";
  printf("%s\n", mma(fuck));
  printf("%s\n", mmb(you));
  return 0;
}

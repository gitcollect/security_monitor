#include <stdlib.h>
#include <stdio.h>

int foo(int *a){
    int b = 3;
    *a = b;
    return 0;
}

int main()
{

  pid_t y = getpid();
  printf("pid: %d\n", y);
  int x = 0;
//  foo(&x);
//  printf("%d\n", x);
  while (1) {
    x ++;
    x --;
  }
  return 0;

}

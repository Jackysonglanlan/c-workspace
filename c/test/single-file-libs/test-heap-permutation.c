
#define HEAPPERMUTE_IMPLEMENTATION
#include "heap-permutation.h"

#include <stdio.h>

byte len;

void print(const int *v)
{
  int i;
  for ( i = 0; i < len; i++) {
    printf("%4d", *(v++) );
  }
  printf("\n");
}

int main(){
   printf("------\n");
   printf("heap-permutation:\n");

   int num[] = {1,2,3};

   len = sizeof(num) / sizeof(int);

   heapPermute(num, len, print);

   return 0;
}

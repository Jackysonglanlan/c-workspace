
#define TINYTIME_IMPLEMENTATION
#include "tinytime.h"

#include <stdio.h>

int main(int argc, char const *argv[])
{

  printf("\n");
  printf("ttTime() - start: %fs\n", ttTime());

  for (int i = 0; i < 1000; ++i)
  {
    /* code */
  }

  printf("ttTime() - end: %fs\n", ttTime());

  return 0;
}

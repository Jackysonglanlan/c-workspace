/*

  Finds all possible permutations of an array for you:

  Here’s an example:

  heapPermute({1, 2, 3}, 3);

  You get:

  [ 1, 2, 3 ]
  [ 2, 1, 3 ]
  [ 3, 1, 2 ]
  [ 1, 3, 2 ]
  [ 2, 3, 1 ]
  [ 3, 2, 1 ]


  This algorithm is "efficient", but still runs in factorial time. It will probably run out of memory
  if you try it on an array longer than 10 items. For example:

    n = 7 will finish within 5 milliseconds (5040 permutations)
    n = 8 will finish within 50 milliseconds (40,320 permutations)
    n = 9 will finish within 500 milliseconds (362,880 permutations)
    n = 10 will finish within 6000 milliseconds (328,800 permutations)

 *
 * See: http://en.wikipedia.org/wiki/Heap's_algorithm
 */


#if !defined( HEAPPERMUTE_H )
#define HEAPPERMUTE_H

typedef unsigned char byte;

// Callback once a single row of the result is calculated.
//
// @param v a single row of the result
typedef void (*HPCallback)(const int *result);

void heapPermute(int v[], byte n, HPCallback cb);

#endif

/*
 * To create implementation (the function definitions)
 *   #define HEAPPERMUTE_IMPLEMENTATION
 * in *one* C/CPP file (translation unit) that includes this file
 */
#ifdef HEAPPERMUTE_IMPLEMENTATION

#include <stdlib.h>

void swap (int *x, int *y){
  int temp;
  temp = *x;
  *x = *y;
  *y = temp;
}

void heapPermute(int v[], byte n, HPCallback cb) {
  byte i;

  if (n == 1) {
    cb(v);
    return;
  }

  for (i = 0; i < n; i++) {
    heapPermute(v, n-1, cb);
    if (n % 2 == 1) {
      swap(&v[0], &v[n-1]);
    }
    else {
      swap(&v[i], &v[n-1]);
    }
  }
}

#endif


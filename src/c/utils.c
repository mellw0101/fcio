/** @file utils.c

  @author  Melwin Svensson.
  @date    14-5-2025.

 */
#include "../include/proto.h"


// void *free_and_assign(void *dest, void *src) {
//   free(dest);
//   return src;
// }

/* Return the number of digits that the given integer n takes up. */
int digits(long n) {
  if (n < 100000) {
    if (n < 1000) {
      if (n < 100) {
        return 2;
      }
      else {
        return 3;
      }
    }
    else {
      if (n < 10000) {
        return 4;
      }
      else {
        return 5;
      }
    }
  }
  else {
    if (n < 10000000) {
      if (n < 1000000) {
        return 6;
      }
      else {
        return 7;
      }
    }
    else {
      if (n < 100000000) {
        return 8;
      }
      else {
        return 9;
      }
    }
  }
}

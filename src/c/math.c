/** @file math.c

  @author  Melwin Svensson.
  @date    10-3-2025.

 */
#include "../include/proto.h"


float fclamp(float x, float min, float max) {
  return ((x > max) ? max : (x < min) ? min : x);
}

long lclamp(long x, long min, long max) {
  return ((x > max) ? max : (x < min) ? min : x);
}

/* Return's the absolute value of `x`. */
float absf(float x) {
  if (x < 0) {
    x *= -1;
  }
  return x;
}

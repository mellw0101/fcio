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

int clamp(int x, int min, int max) {
  return ((x > max) ? max : (x < min) ? min : x);
}

int clamp_min(int x, int min) {
  return ((x < min) ? min : x);
}

int clamp_max(int x, int max) {
  return ((x > max) ? max : x);
}

/* ----------------------------- luclamp ----------------------------- */

Ulong luclamp(Ulong x, Ulong min, Ulong max) {
  return ((x > max) ? max : (x < min) ? min : x);
}

Ulong luclamp_min(Ulong x, Ulong min) {
  return ((x < min) ? min : x);
}

Ulong luclamp_max(Ulong x, Ulong max) {
  return ((x > max) ? max : x);
}

/* ----------------------------- zuclamp ----------------------------- */

size_t zuclamp(size_t x, size_t min, size_t max) {
  return ((x > max) ? max : (x < min) ? min : x);
}

size_t zuclamp_min(size_t x, size_t min) {
  return ((x < min) ? min : x);
}

size_t zuclamp_max(size_t x, size_t max) {
  return ((x > max) ? max : x);
}

/* Return's the absolute value of `x`. */
float absf(float x) {
  if (x < 0) {
    x *= -1;
  }
  return x;
}

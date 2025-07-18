/** @file statics.h

  @author  Melwin Svensson.
  @date    15-7-2025.

 */
#pragma once
#include "proto.h"


/* ---------------------------------------------------------- Cpu function's ---------------------------------------------------------- */


static inline bool has_inveriant_tsc(void) {
  Uint eax = 0x80000007;
  Uint ebx;
  Uint ecx;
  Uint edx;
  __asm__ __volatile__(
    "cpuid"
    : "+a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
  );
  return !!(edx & (1 << 8));
}


/* ---------------------------------------------------------- Math function's ---------------------------------------------------------- */



/* ----------------------------- Bazier point ----------------------------- */

static inline void bazier_point(float x0, float y0, float x1, float y1, float ax, float ay, float t, float *const px, float *const py) {
  *px = ((x0 * ((1 - t) * (1 - t))) + (ax * (2 * (1 - t) * t)) + (x1 * (t * t)));
  *py = ((y0 * ((1 - t) * (1 - t))) + (ay * (2 * (1 - t) * t)) + (y1 * (t * t)));
}

/* ----------------------------- Bazier arc ----------------------------- */

static inline void bazier_arc(float x0, float y0, float x1, float y1, float ax, float ay, float *const arcx, float *const arcy, Ulong n) {
  float t;
  for (Ulong i=0; i<n; ++i) {
    t = ((float)i / (n - 1));
    bazier_point(x0, y0, x1, y1, ax, ay, t, (arcx + i), (arcy + i));
  }
}

/* ----------------------------- Fcenter ----------------------------- */

/* Returns the center point between `x` and `y`. */
static inline float fcenter(float x, float y) {
  return ((x + y) / 2);
}

/* ----------------------------- Fcentercenter ----------------------------- */

/* Returns the center point between `x` and the center point of `x` and `y`. */
static inline float fcentercenter(float x, float y) {
  return fcenter(x, fcenter(x, y));
}

/* ----------------------------- Fsquare ----------------------------- */

/* Returns the square of `x`. */
static inline float fsq(float x) {
  return (x * x);
}

/* ----------------------------- Fdeg ----------------------------- */

static inline float fdeg(float x) {
  return (x * (180.f / M_PIf));
}

/* ----------------------------- Frad ----------------------------- */

static inline float frad(float x) {
  return (x * (M_PIf / 180.f));
}

/* ----------------------------- Fangle_to ----------------------------- */

static inline float fangle_to(float x0, float y0, float x1, float y1) {
  return fdeg(ATAN2F((y1 - y0), (x1 - x0)));
}

/* ----------------------------- Fmagnitude ----------------------------- */

/* Returns the length from [0,0] to [x,y]. */
static inline float fmag(float x, float y) {
  return SQRTF(fsq(x) + fsq(y));
}

/* ----------------------------- Fvec2 center ----------------------------- */

static inline void fvec2_center(float x0, float y0, float x1, float y1, float *const cx, float *const cy) {
  ASSERT(cx);
  ASSERT(cy);
  *cx = fcenter(x0, x1);
  *cy = fcenter(y0, y1);
}

/* ----------------------------- Flerp ----------------------------- */

static inline void flerp(float t, float x0, float y0, float x1, float y1, float *const px, float *const py) {
  *px = (x0 + (t * (x1 - x0)));
  *py = (y0 + (t * (y1 - y0)));
}

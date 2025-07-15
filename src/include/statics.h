/** @file statics.h

  @author  Melwin Svensson.
  @date    15-7-2025.

 */
#include "proto.h"


/* ---------------------------------------------------------- Global function's ---------------------------------------------------------- */


/* ----------------------------- Cpu ----------------------------- */

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


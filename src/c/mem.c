/** @file mem.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"


/* Return's a allocated ptr of size `howmush`.  Note that this function can never return an invalid ptr. */
void *xmalloc(Ulong howmush) {
  void *ptr = malloc(howmush);
  ALWAYS_ASSERT(ptr);
  return ptr;
}

/* Return's `ptr` reallocated to size `newsize`.  Note that this function can never return an invalid ptr. */
void *xrealloc(void *ptr, Ulong newsize) {
  ptr = realloc(ptr, newsize);
  ALWAYS_ASSERT(ptr);
  return ptr;
}

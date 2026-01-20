/** @file mem.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#define _USE_ALL_BUILTINS
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

/* Return's a ptr with the total size `n * size`. */
void *xcalloc(Ulong n, Ulong size) {
  void *ptr = calloc(n, size);
  ALWAYS_ASSERT(ptr);
  return ptr;
}

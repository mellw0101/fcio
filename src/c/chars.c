/** @file chars.c

  @author  Melwin Svensson.
  @date    6-3-2025.

 */
#include "../include/proto.h"


/* Return's `TRUE` if `c` is one of the chars in `string`. */
bool isconeof(const char c, const char *const restrict string) {
  return (strchr(string, c));
}

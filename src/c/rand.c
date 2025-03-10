/** @file rand.c

  @author  Melwin Svensson.
  @date    10-3-2025.

 */
#include "../include/proto.h" 


/* Return's a allocated string of the given `length`.  Note that this string will only contain alphanumaric char's. */
char *randstr(Ulong length) {
  ASSERT(length);
  /* The static array of chars that will be used to construct the return string. */
  static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  /* Allocate the return string to the length plus space for the `NULL-TERMINATOR`. */
  char *ret = xmalloc(length + 1);
  for (Ulong i=0; i<length; ++i) {
    ret[i] = charset[rand() % STRLEN(charset)];
  }
  /* `NULL-TERMINATE` the return string. */
  ret[length] = '\0';
  return ret;
}

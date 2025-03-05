/** @file str.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"


/* Return's a allocated `len` copy of `string`.  Note that this function cannot return an invalid ptr. */
char *measured_copy(const char *const restrict string, Ulong len) {
  ASSERT(string);
  char *ret = xmalloc(len + 1);
  memcpy(ret, string, len);
  ret[len] = '\0';
  return ret;
}

/* Return's a allocated copy of `string`.  Note that this function cannot return an invalid ptr. */
char *copy_of(const char *const restrict string) {
  ASSERT(string);
  return measured_copy(string, strlen(string));
}

/* Return's a allocated formated string.  Note that this function cannot return an invalid ptr. */
char *fmtstr(const char *const restrict format, ...) {
  ASSERT(format);
  /* The ptr we will allocate and return. */
  char *ret;
  /* The length of the fully formated string. */
  int len;
  /* Veriatic arguments list we will use to format the string. */
  va_list ap;
  /* Veriatic arguments list we will use to fetch the length of the fully formatted string. */
  va_list dummy;
  /* Get the length we need to allocate. */
  va_start(dummy, format);
  ALWAYS_ASSERT((len = vsnprintf(NULL, 0, format, dummy)) != -1);
  va_end(dummy);
  /* Allocate the return ptr to the correct len. */
  ret = xmalloc(len + 1);
  /* Format the string into ret. */
  va_start(ap, format);
  ALWAYS_ASSERT(vsnprintf(ret, (len + 1), format, ap) != -1);
  va_end(ap);
  return ret;
}

/* Create a allocated string from veriatic arguments, and assign the length to `*outlen`. */
char *valstr(const char *const restrict format, va_list ap, int *const outlen) {
  ASSERT(format);
  char *ret;
  int len;
  va_list copy;
  va_copy(copy, ap);
  ALWAYS_ASSERT((len = vsnprintf(NULL, 0, format, copy)) != -1);
  va_end(copy);
  ret = xmalloc(len + 1);
  ALWAYS_ASSERT(vsnprintf(ret, (len + 1), format, ap) != -1);
  ASSIGN_IF_VALID(outlen, len);
  return ret;
}

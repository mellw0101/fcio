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

/* Split a string by demiliter. */
char **split_string(const char *const restrict string, const char delim) {
  ASSERT(string);
  ASSERT(delim);
  /* Initilaze both start and end to string. */
  const char *start = string;
  const char *end   = string;
  /* Set up the return array. */
  Ulong  cap = 10;
  Ulong  len = 0;
  char **result = xmalloc(_PTRSIZE * cap);
  /* Iterate until the end of the string. */
  while (*end) {
    /* Advance end until we se the delimiter. */
    while (*end && *end != delim) {
      ++end;
    }
    ENSURE_PTR_ARRAY_SIZE(result, cap, len);
    result[len++] = measured_copy(start, (end - start));
    /* Break if we reached the end of the string. */
    if (!*end) {
      break;
    }
    /* Advance end past all delim, for instance if delim
     * is ' ' then double space is just advanced past. */
    while (*end && *end == delim) {
      ++end;
    }
    /* Set start to the first char that is not delim. */
    start = end;
  }
  /* Trim the array before returning it, saving memory where we can. */
  TRIM_PTR_ARRAY(result, cap, len);
  result[len] = NULL;
  return result;
}

/* Get a integer representation of `string`. */
long strtonum(const char *const restrict string) {
  ASSERT(string);
  ASSERT(*string);
  long ret;
  char *endptr;
  /* Set errno to zero, so we can check it later. */
  errno = 0;
  ret = strtol(string, &endptr, 10);
  /* Check for range errors. */
  ALWAYS_ASSERT_MSG((errno != ERANGE), "Return'ed value is out of range");
  /* Check that the string was read correctly. */
  ALWAYS_ASSERT_MSG(!*endptr, "Passed string included somthing other then numbers");
  return ret;
}

/* ----------------------------- xstrcat ----------------------------- */

/* Append `src` to the end of `dst`. */
char *xnstrncat(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong srclen) {
  ASSERT(dst);
  ASSERT(src);
  /* Reallocate dst to fit all of the text plus a NULL-TERMINATOR. */
  dst = xrealloc(dst, (dstlen + srclen + 1));
  /* Append src to dst. */
  memcpy((dst + dstlen), src, srclen);
  /* Null terminate the string. */
  dst[dstlen + srclen] = '\0';
  /* Then return dst. */
  return dst;
}

/* Append `src` to the end of `dst`. */
char *xnstrcat(char *restrict dst, Ulong dstlen, const char *const restrict src) {
  return xnstrncat(dst, dstlen, src, strlen(src));
}

/* Append `src` to the end of `dst`. */
char *xstrncat(char *restrict dst, const char *const restrict src, Ulong srclen) {
  return xnstrncat(dst, strlen(dst), src, srclen);
}

/* Append `src` to the end of `dst`. */
char *xstrcat(char *restrict dst, const char *const restrict src) {
  return xnstrncat(dst, strlen(dst), src, strlen(src));
}

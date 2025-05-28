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
  /* If the user wants the total length of the formated string, then assign it to `*outlen`. */
  ASSIGN_IF_VALID(outlen, len);
  return ret;
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

/* Parse a number from a string. */
bool parse_num(const char *const restrict string, long *const result) {
  long value;
  char *excess;
  /* Clear errno so we can check it after. */
  errno = 0; 
  value = strtol(string, &excess, 10);
  if (errno == ERANGE || !*string || *excess) {
    return FALSE;
  }
  *result = value;
  return TRUE;
}

/* Free the string at `dest` and return the string at `src`. */
char *free_and_assign(char *dest, char *src) {
  free(dest);
  return src;
}

/* ----------------------------- split_string ----------------------------- */

/* Split a string by demiliter. */
char **split_string_len(const char *const restrict string, const char delim, Ulong *const argc) {
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
  /* If the user wants it, assign the lenth of the array to `*argc`. */
  ASSIGN_IF_VALID(argc, len);
  return result;
}

/* Split a string by demiliter. */
char **split_string(const char *const restrict string, const char delim) {
  return split_string_len(string, delim, NULL);
}

/* ----------------------------- chararray ----------------------------- */

/* Free the memory of the given array, which should contain len elements. */
void chararray_free(char **const restrict array, Ulong len) {
  /* Make this function a 'no-op' function. */
  if (!array) {
    return;
  }
  /* Free every entry in the array. */
  while (len > 0) {
    free(array[--len]);
  }
  free(array);
}

/* Free a `NULL-TERMINATED` char array. */
void chararray_freenullterm(char **const restrict array) {
  ASSERT(array);
  char **a = array;
  /* Make this function a `NO-OP` function. */
  if (!a) {
    return;
  }
  /* Iterate until we reach the `NULL-TERMINATOR`. */
  while (*a) {
    free(*a);
    ++a;
  }
  /* Then free the array ptr itself. */
  free(array);
}

/* Append an array onto 'array'.  Free 'append' but not any elements in it after call. */
void chararray_append(char ***array, Ulong *const len, char **append, Ulong append_len) {
  ASSERT(array);
  ASSERT(append);
  /* Calculate the new total length for the array. */
  Ulong new_len = ((*len) + append_len);
  /* Reallocate the array ptr so that it can hold the new total length plus a `NULL-TERMINATOR`. */
  *array = xrealloc(*array, (_PTRSIZE * (new_len + 1)));
  /* Append all entries. */
  memcpy(((*array) + (*len)), append, (_PTRSIZE * append_len));
  /* Set the len ptr to the new total length. */
  *len = new_len;
  /* `NULL-TERMINATE` the array. */
  (*array)[*len] = NULL;
}

/* Remove one entry from `array` at `idx`. */
void chararray_erase(char **const array, Ulong *len, Ulong idx) {
  ASSERT(array);
  ASSERT(len);
  ALWAYS_ASSERT(idx < *len);
  /* Free the string at index. */
  free(*(array + idx));
  /* When the erased entry was not the last, move the remaining entries in the array. */
  if (idx != (*len - 1)) {
    memmove((array + idx), (array + (idx + 1)), (_PTRSIZE * ((*len) - idx)));
  }
  /* Remove one from length so it accuretly reflects the new length of the array. */
  *len -= 1;
  *(array + (*len)) = NULL;
}

/* ----------------------------- fmtstr ----------------------------- */

/* Return's a allocated formated string.  Note that this function cannot return an invalid ptr. */
char *fmtstr(const char *const restrict format, ...) {
  ASSERT(format);
  /* The ptr we will allocate and return. */
  char *ret;
  /* Veriatic arguments list we will use to format the string. */
  va_list ap;
  /* Now form the return string. */
  va_start(ap, format);
  ret = valstr(format, ap, NULL);
  va_end(ap);
  return ret;
}

/* Return's a allocated formated string.  Note that this function cannot return an invalid ptr. */
char *fmtstr_len(int *const fmtlen, const char *const restrict format, ...) {
  /* Assert 'fmtlen' because if the length is not needed 'fmtstr()' should be used. */
  ASSERT(fmtlen);
  ASSERT(format);
  /* The ptr we will allocate and return. */
  char *ret;
  /* Veriatic arguments list we will use to format the string. */
  va_list ap;
  /* Format the string into ret. */
  va_start(ap, format);
  ret = valstr(format, ap, fmtlen);
  va_end(ap);
  return ret;
}

/* ----------------------------- fmtstrcat ----------------------------- */

/* Append a format'ed string to `dst`. */
char *fmtstrncat(char *restrict dst, Ulong dstlen, const char *const restrict format, ...) {
  ASSERT(dst);
  ASSERT(format);
  int srclen;
  char *src;
  va_list ap;
  va_start(ap, format);
  src = valstr(format, ap, &srclen);
  va_end(ap);
  dst = xnstrncat(dst, dstlen, src, srclen);
  free(src);
  return dst;
}

/* Append a format'ed string to `dst`. */
char *fmtstrcat(char *restrict dst, const char *const restrict format, ...) {
  ASSERT(dst);
  ASSERT(format);
  int srclen;
  char *src;
  va_list ap;
  va_start(ap, format);
  src = valstr(format, ap, &srclen);
  va_end(ap);
  dst = xstrncat(dst, src, srclen);
  free(src);
  return dst;
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

/* ----------------------------- xstrinj ----------------------------- */

/* Inject `src` into `dst` at index `idx`. */
char *xnstrninj_norealloc(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong srclen, Ulong idx) {
  ASSERT(dst);
  ASSERT(src);
  /* Always assert that idx is valid. */
  ALWAYS_ASSERT(idx <= dstlen);
  /* First move the data at idx by srclen. */
  memmove((dst + idx + srclen), (dst + idx), (dstlen - idx));
  /* Then copy src into dst at idx. */
  memcpy((dst + idx), src, srclen);
  /* Explicitly set the `null-terminator`, this way it does not matter if `dstlen` is the full length of `dst`. */
  dst[dstlen + srclen] = '\0';
  return dst;
}

/* Inject `src` into `dst` at index `idx`. */
char *xnstrinj_norealloc(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong idx) {
  return xnstrninj_norealloc(dst, dstlen, src, strlen(src), idx);
}

/* Inject `src` into `dst` at index `idx`. */
char *xstrninj_norealloc(char *restrict dst, const char *const restrict src, Ulong srclen, Ulong idx) {
  return xnstrninj_norealloc(dst, strlen(dst), src, srclen, idx);
}

/* Inject `src` into `dst` at index `idx`. */
char *xstrinj_norealloc(char *restrict dst, const char *const restrict src, Ulong idx) {
  return xnstrninj_norealloc(dst, strlen(dst), src, strlen(src), idx);
}

/* Inject `src` into `dst` at index `idx`. */
char *xnstrninj(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong srclen, Ulong idx) {
  ASSERT(dst);
  ASSERT(src);
  /* Always assert that idx is valid. */
  ALWAYS_ASSERT(idx <= dstlen);
  /* Reallocate dst to fit src and a NULL-TERMINATOR. */
  dst = xrealloc(dst, (dstlen + srclen + 1));
  /* First move the data at idx by srclen. */
  memmove((dst + idx + srclen), (dst + idx), (dstlen - idx));
  /* Then copy src into dst at idx. */
  memcpy((dst + idx), src, srclen);
  /* Explicitly set the `null-terminator`, this way it does not matter if `dstlen` is the full length of `dst`. */
  dst[dstlen + srclen] = '\0';
  return dst;
}

/* Inject `src` into `dst` at index `idx`. */
char *xnstrinj(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong idx) {
  return xnstrninj(dst, dstlen, src, strlen(src), idx);
}

/* Inject `src` into `dst` at index `idx`. */
char *xstrninj(char *restrict dst, const char *const restrict src, Ulong srclen, Ulong idx) {
  return xnstrninj(dst, strlen(dst), src, srclen, idx);
}

/* Inject `src` into `dst` at index `idx`. */
char *xstrinj(char *restrict dst, const char *const restrict src, Ulong idx) {
  return xnstrninj(dst, strlen(dst), src, strlen(src), idx);
}

/* ----------------------------- xstr_erase ----------------------------- */

/* Erase `len` of `dst` at `index`.  Note that this function does `NOT` reallocate `dst`, it just `null-terminates` at the new length. */
char *xstrn_erase_norealloc(char *restrict dst, Ulong dstlen, Ulong index, Ulong len) {
  ASSERT(dst);
  ALWAYS_ASSERT((index + len) <= dstlen);
  /* Move data from `index + len` to `index` in `dst`. */
  memmove((dst + index), (dst + index + len), (dstlen - index - len));
  /* Then explicitly `null-terminate` `dst`. */
  dst[dstlen - len] = '\0';
  return dst;
}

/* Erase `len` of `dst` at `index`.  Note that this function does `NOT` reallocate `dst`, it just `null-terminates` at the new length. */
char *xstr_erase_norealloc(char *restrict dst, Ulong index, Ulong len) {
  return xstrn_erase_norealloc(dst, strlen(dst), index, len);
}

/* Erase `len` of `dst` at `index`. */
char *xstrn_erase(char *restrict dst, Ulong dstlen, Ulong index, Ulong len) {
  ASSERT(dst);
  ALWAYS_ASSERT((index + len) <= dstlen);
  /* Move data from `index + len` to `index` in `dst`. */
  memmove((dst + index), (dst + index + len), (dstlen - index - len));
  /* Reallocate `dst` to fit just the new length plus a `null-terminator.` */
  dst = xrealloc(dst, (dstlen - len + 1));
  /* Then explicitly `null-terminate` `dst`. */
  dst[dstlen - len] = '\0';
  return dst;
}

/* Erase `len` of `dst` at `index`. */
char *xstr_erase(char *restrict dst, Ulong index, Ulong len) {
  return xstrn_erase(dst, strlen(dst), index, len);
}

/** @file chars.c

  @author  Melwin Svensson.
  @date    6-3-2025.

 */
#include "../include/proto.h"


static bool using_utf8 = FALSE;


void initcheck_utf8(void) {
  /* If setting the locale is successful and it uses UTF-8, we will need to use the multibyte functions for text processing. */
  if (setlocale(LC_ALL, "") && strcmp(nl_langinfo(CODESET), "UTF-8") == 0) {
    using_utf8 = TRUE;
  }
}

/* Return the length of a utf8 multibyte char. */
int charlen(const char *const restrict ptr) {
  ASSERT(ptr);
  Uchar c1, c0=(Uchar)ptr[0];
  /* Check if ptr starts with a valid utf9-char and if utf8 is enabled. */
  if (c0 > 0xC1 && using_utf8) {
    c1 = (Uchar)ptr[1];
    if ((c1 ^ 0x80) > 0x3F) {
      return 1;
    }
    else if (c0 < 0xE0) {
      return 2;
    }
    if (((Uchar)ptr[2] ^ 0x80) > 0x3F) {
      return 1;
    }
    else if (c0 < 0xF0) {
      if ((c0 > 0xE0 || c1 >= 0xA0) && (c0 != 0xED || c1 < 0xA0)) {
        return 3;
      }
      return 1;
    }
    if (((Uchar)ptr[3] ^ 0x80) > 0x3F || c0 > 0xF4) {
      return 1;
    }
    else if ((c0 > 0xF0 || c1 >= 0x90) && (c0 != 0xF4 || c1 < 0x90)) {
      return 4;
    }
  }
  /* Otherwise just return a length of one. */
  return 1;
}

/* ----------------------------- Boolian char checks ----------------------------- */

/* Return's `TRUE` if `c` is one of the chars in `string`. */
bool isconeof(const char c, const char *const restrict string) {
  return (strchr(string, c));
}

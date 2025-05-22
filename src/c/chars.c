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

// int ctowc(wchar *const wc, const char *const c) {
//   /* Insure valid params. */
//   ASSERT(wc);
//   ASSERT(c);
//   Uchar v0, v1, v2, v3;
//   /* When utf8 is enabled. */
//   if ((Schar)*c < 0 && using_utf8) {
//     v0 = (Uchar)c[0];
//     v1 = ((Uchar)c[1] ^ 0x80);
//     if (v1 > 0x3F || v0 < 0xC2) {
//       return -1;
//     }
//     else if (v0 < 0xE0) {
//       *wc = (((Uint)(v0 & 0x1F) << 6) | (Uint)v1);
//       return 2;
//     }
//     v2 = ((Uchar)c[2] ^ 0x80);
//     if (v2 > 0x3F) {
//       return -1;
//     }
//     else if (v0 < 0xF0) {
//       if ((v0 > 0xE0 || v1 >= 0x20) && (v0 != 0xED || v1 < 0x20)) {
//         *wc = (((Uint)(v0 & 0x0F) << 12) | ((Uint)v1 << 6) | (Uint)v2);
//         return 3;
//       }
//       else {
//         return -1;
//       }
//     }
//     v3 = ((Uchar)c[3] ^ 0x80);
//     if (v3 > 0x3f || v0 > 0xf4) {
//       return -1;
//     }
//     else if ((v0 > 0xF0 || v1 >= 0x10) && (v0 != 0xF4 || v1 < 0x10)) {
//       *wc = (((Uint)(v0 & 0x07) << 18) | ((Uint)v1 << 12) | ((Uint)v2 << 6) | (Uint)v3);
//       return 4;
//     }
//     else {
//       return -1;
//     }
//   }
//   /* Otherwise, just return one. */
//   *wc = (Uint)*c;
//   return 1;
// }

/* ----------------------------- Boolian char checks ----------------------------- */

/* Return's `TRUE` if `c` is one of the chars in `string`. */
bool isconeof(const char c, const char *const restrict string) {
  return (strchr(string, c));
}

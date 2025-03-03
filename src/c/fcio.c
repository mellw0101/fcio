/** @file fcio.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"


_NO_RETURN static void fcio_default_die_callback(const char *format, ...)  {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  exit(1);
}
void (*die_callback)(const char *format, ...) = fcio_default_die_callback;


/* Set the function that will be called when a fatal error happens. */
void fcio_set_die_callback(void (*callback)(const char *format, ...) _NO_RETURN) {
  die_callback = (callback ? callback : fcio_default_die_callback);
}

/** @file fcio.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"



/* ----------------------------- Variable decl's ----------------------------- */

static mutex_t stdout_mutex = mutex_init_static;

/* ----------------------------- Die callback ----------------------------- */

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

/* ----------------------------- Thread safe stdout ----------------------------- */

/* Write `len` of `data` to stdout in a fully thread and process safe manner. */
void stdoutwrite(const char *const restrict data, Ulong len) {
  ASSERT(data);
  mutex_action(&stdout_mutex, fdlock_action(STDOUT_FILENO, F_WRLCK,
    ALWAYS_ASSERT(write(STDOUT_FILENO, data, len) != -1);  
  ););
}

/* Write a formated string to stdout in a thread and process safe manner. */
void writef(const char *const restrict format, ...) {
  ASSERT(format);
  int len;
  char *string;
  va_list ap;
  /* Format the string. */
  va_start(ap, format);
  string = valstr(format, ap, &len);
  va_end(ap);
  /* Write the formated string to stdout in a fully thread and process safe mannor. */
  stdoutwrite(string, len);
  /* Free the formated string. */
  free(string);
}

/* Same as `writef()`, but takes a `va_list` directly as a parameter. */
void vwritef(const char *const restrict format, va_list ap) {
  ASSERT(format);
  int len;
  /* Get the formated string. */
  char *string = valstr(format, ap, &len);
  /* Write the string to stdout. */
  stdoutwrite(string, len);
  /* Free the allocated string. */
  free(string);
}

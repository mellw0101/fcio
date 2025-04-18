/** @file fcio.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"


/* ----------------------------- Variable decl's ----------------------------- */

static mutex_t stdout_mutex = mutex_init_static;
static mutex_t stderr_mutex = mutex_init_static;

/* ----------------------------- Die callback ----------------------------- */

/* The default die callback when the user has not set one. */
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

/* Write `len` of `data` to stderr in a fully thread and process safe manner. */
void stderrwrite(const char *const restrict data, Ulong len) {
  ASSERT(data);
  mutex_action(&stderr_mutex, fdlock_action(STDERR_FILENO, F_WRLCK,
    ALWAYS_ASSERT(write(STDERR_FILENO, data, len) != -1);
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

/* Write a formated string to stderr in a thread and process safe manner. */
void writeferr(const char *const restrict format, ...) {
  ASSERT(format);
  int len;
  char *string;
  va_list ap;
  /* Format the string. */
  va_start(ap, format);
  string = valstr(format, ap, &len);
  va_end(ap);
  /* Write the formated string to stderr. */
  stderrwrite(string, len);
  /* Free the formatted string. */
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

/* Same as `writeferr()`, but takes a `va_list` directly as a parameter. */
void vwriteferr(const char *const restrict format, va_list ap) {
  ASSERT(format);
  int len;
  /* Get the formatted string. */
  char *string = valstr(format, ap, &len);
  /* Write the string to stderr. */
  stderrwrite(string, len);
  /* Free the formatted string. */
  free(string);
}

/* Return's `TRUE` when user answer's `Y/y` and `FALSE` when user answer's `N/n`.  Note that this function only accepts Y/y and N/n answers and returns directly on input. */
bool ynanswer(const char *const restrict format, ...) {
  ASSERT(format);
  struct termios oldt, newt;
  int oldf;
  char c;
  bool ret;
  va_list ap;
  /* Write the format string to stdout. */
  va_start(ap, format);
  vwritef(format, ap);
  va_end(ap);
  /* Get the current termios. */
  ALWAYS_ASSERT(tcgetattr(STDIN_FILENO, &oldt) != -1);
  newt = oldt;
  /* Disable canonical mode and echo. */
  newt.c_lflag &= ~(ICANON | ECHO);
  ALWAYS_ASSERT(tcsetattr(STDIN_FILENO, TCSANOW, &newt) != -1);
  /* Set stdin to non blocking mode. */
  ALWAYS_ASSERT((oldf = fcntl(STDIN_FILENO, F_GETFL, 0)) != -1);
  ALWAYS_ASSERT(fcntl(STDIN_FILENO, F_SETFL, (oldf | O_NONBLOCK)) != -1);
  /* Only accept a 'Yy/Nn' char. */
  while (TRUE) {
    if (read(STDIN_FILENO, &c, 1) > 0) {
      /* If user ansers yes set return to TRUE. */
      if (isconeof(c, "Yy")) {
        ret = TRUE;
        break;
      }
      /* Otherwise, if user answers no then set return to FALSE. */
      else if (isconeof(c, "Nn")) {
        ret = FALSE;
        break;
      }
    }
  }
  /* Print the choice the user made. */
  writef("%c\n", c);
  /* Restore terminal state. */
  ALWAYS_ASSERT(tcsetattr(STDIN_FILENO, TCSANOW, &oldt) != -1);
  ALWAYS_ASSERT(fcntl(STDIN_FILENO, F_SETFL, oldf) != -1);
  return ret;
}

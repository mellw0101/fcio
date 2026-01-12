/** @file log.c

  @author  Melwin Svensson.
  @date    19-7-2025.

 */
#include "../include/proto.h"

#if !__WIN__

/* ---------------------------------------------------------- Define's ---------------------------------------------------------- */


#define LOG_TAG(x)          fcio_log_type_tag[(x)]
#define LOG_COLOR_START(x)  fcio_log_type_color_start[(x)]
#define LOG_COLOR_END(x)    fcio_log_type_color_end[(x)]


/* ---------------------------------------------------------- Enum's ---------------------------------------------------------- */


/* Logging type.  Note that these are not in any perticular order. */
typedef enum {
  FCIO_LOG_INFO_0,
  FCIO_LOG_INFO_1,
  FCIO_LOG_WARN_0,
  FCIO_LOG_ERR_NF,
  FCIO_LOG_ERR_FA
# define FCIO_LOG_INFO_0    \
  /* Low prio info log. */  \
  FCIO_LOG_INFO_0
# define FCIO_LOG_INFO_1    \
  /* Medium prio info log. */  \
  FCIO_LOG_INFO_1
# define FCIO_LOG_WARN_0  \
  /* Low prio warning log */ \
  FCIO_LOG_WARN_0
# define FCIO_LOG_ERR_NF  \
  /* Non fatal error. */  \
  FCIO_LOG_ERR_NF
# define FCIO_LOG_ERR_FA      \
  /* Fatal error logging. */  \
  FCIO_LOG_ERR_FA

# define FCIO_LOG_TYPE_FIRST  FCIO_LOG_INFO_0
# define FCIO_LOG_TYPE_LAST   FCIO_LOG_ERR_FA
} FcioLogType;


/* ---------------------------------------------------------- Variable's ---------------------------------------------------------- */


static const char *const fcio_log_type_tag[FCIO_LOG_TYPE_LAST + 1] = {
  "INFO_0",
  "INFO_1",
  "WARN_0",
  "ERR_NF",
  "ERR_FA"
};

static const char *const fcio_log_type_color_start[FCIO_LOG_TYPE_LAST + 1] = {
  "\033[90m",         /* INFO_0 */
  "\033[1m\033[94m",  /* INFO_1 */
  "\033[33m",         /* WARN_0 */
  "\033[31m",         /* ERR_NF */
  "\033[1m\033[31m",  /* ERR_FA */
};

static const char *const fcio_log_type_color_end[FCIO_LOG_TYPE_LAST + 1] = {
  "\033[0m",  /* INFO_0 */
  "\033[0m",  /* INFO_1 */
  "\033[0m",  /* WARN_0 */
  "\033[0m",  /* ERR_NF */
  "\033[0m",  /* ERR_FA */
};

static mutex_t fcio_log_mutex = mutex_init_static;
static int fcio_log_fd = -1;


/* ---------------------------------------------------------- Static function's ---------------------------------------------------------- */


/* ----------------------------- Fcio log va ----------------------------- */

static void fcio_log_va(int type, Ulong lineno,
  const char *const restrict function, const char *const restrict format, va_list ap)
{
  ASSERT(format);
  ASSERT(type >= FCIO_LOG_TYPE_FIRST && type <= FCIO_LOG_TYPE_LAST);
  bool log_to_std = TRUE;
  va_list copy;
  char *log;
  char *data;
  int datalen;
  long len;
  long written = 0;
  va_copy(copy, ap);
  log = valstr(format, copy, NULL);
  va_end(copy);
  MUTEX_ACTION(&fcio_log_mutex,
    if (fcio_log_fd != -1) {
      log_to_std = FALSE;
    }
  );
  data = fmtstr_len(
    &datalen,
    "[%s]:[LINE]:[%lu]%*s:[FUNC]:[%s]: %s\n",
    LOG_TAG(type),
    lineno,
    ((digits(lineno) < 5) ? (5 - digits(lineno)) : 0),
    " ",
    PASS_IF_VALID(function, "GLOBAL"),
    log
  );
  free(log);
  /* Only when logging to std out/err do we color the text using ascii esc codes. */
  if (log_to_std) {
    if (type >= FCIO_LOG_ERR_NF) {
      writeferr("%s%s%s", LOG_COLOR_START(type), data, LOG_COLOR_END(type));
    }
    else {
      writef("%s%s%s", LOG_COLOR_START(type), data, LOG_COLOR_END(type));
    }
  }
  else {
    mutex_fdlock_full_wr(&fcio_log_mutex, fcio_log_fd, written, len, data, datalen, TRUE);
  }
  free(data);
}


/* ---------------------------------------------------------- Global function's ---------------------------------------------------------- */


/* ----------------------------- Fcio log set file ----------------------------- */

void fcio_log_set_file(const char *const restrict path) {
  ASSERT(path);
  struct stat info;
  if (access(path, F_OK) != 0) {
    die_callback("Cannot access '%s'.  Check permissions.\n", path);
  }
  else if (access(path, W_OK) != 0) {
    die_callback("Cannot write to '%s'.  Check permissions.\n", path);
  }
  else if (stat(path, &info) == -1) {
    die_callback("Cannot write to '%s'.\n", path);
  }
  else {
    /* Block-Device */
    if (S_ISBLK(info.st_mode)) {
      die_callback("Cannot write to block device '%s'.\n", path);
    }
    /* Directory */
    else if (S_ISDIR(info.st_mode)) {
      die_callback("Cannot write to directory '%s'.\n", path);
    }
    else if (S_ISCHR(info.st_mode)) {
      die_callback("Cannot write to '%s'.\n", path);
    }
    /* If this is a good path, then open the file-descriptor. */
    else {
      mutex_action(&fcio_log_mutex,
        if ((fcio_log_fd = open(path, O_WRONLY)) == -1) {
          die_callback("Failed to open '%s': %s\n", path, strerror(errno));
        }
      );
    }
  } 
}

/* ----------------------------- Fcio log ----------------------------- */

void fcio_log(int type, Ulong lineno, const char *const restrict function, const char *const restrict format, ...) {
  ASSERT(format);
  va_list ap;
  va_start(ap, format);
  fcio_log_va(type, lineno, function, format, ap);
  va_end(ap);
}

/* ----------------------------- Fcio log error fatal ----------------------------- */

void fcio_log_error_fatal(Ulong lineno, const char *const restrict function, const char *const restrict format, ...) {
  ASSERT(format);
  va_list ap;
  va_start(ap, format);
  fcio_log_va(FCIO_LOG_ERR_FA, lineno, function, format, ap);
  va_end(ap);
  die_callback("\nTERMINATING: The last log was a fatal error.\n");
}

#endif

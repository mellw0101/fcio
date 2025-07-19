/** @file log.c

  @author  Melwin Svensson.
  @date    19-7-2025.

 */
#include "../include/proto.h"


/* ---------------------------------------------------------- Enum's ---------------------------------------------------------- */


/* Logging type.  Note that these are not in any perticular order. */
typedef enum {
  FCIO_LOG_INFO_0,
  FCIO_LOG_WARN_0,
  FCIO_LOG_ERR_NF,
  FCIO_LOG_ERR_FA
# define FCIO_LOG_INFO_0    \
  /* Low prio info log. */  \
  FCIO_LOG_INFO_0
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


static const char *const fcio_log_type_tag[] = {
  "INFO_0",
  "WARN_0",
  "ERR_NF",
  "ERR_FA"
};

static mutex_t fcio_log_mutex = mutex_init_static;
static int fcio_log_fd = -1;


/* ---------------------------------------------------------- Static function's ---------------------------------------------------------- */


/* ----------------------------- Fcio log va ----------------------------- */

static void fcio_log_va(int type, Ulong lineno, const char *const restrict function, const char *const restrict format, va_list ap) {
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
  mutex_action(&fcio_log_mutex,
    if (fcio_log_fd != -1) {
      log_to_std = FALSE;
    }
  );
  data = fmtstr_len(&datalen, "%s: %lu: %s: %s\n", PASS_IF_VALID(function, "GLOBAL"), lineno, fcio_log_type_tag[type], log);
  free(log);
  if (log_to_std) {
    if (type >= FCIO_LOG_ERR_NF) {
      writeferr("%s", data);
    }
    else {
      writef("%s", data);
    }
  }
  else {
    mutex_fdlock_full_wr(&fcio_log_mutex, fcio_log_fd, written, len, data, datalen, TRUE);
  }
  free(data);
  /* If this was a fatal error. */
  if (type == FCIO_LOG_ERR_FA) {
    die_callback("\nTERMINATING: The last log was a fatal error.\n");
  }
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

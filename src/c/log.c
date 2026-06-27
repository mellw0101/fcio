/** @file log.c

  @author  Melwin Svensson.
  @date    19-7-2025.

 */
#define _USE_ALL_BUILTINS
#include "../include/proto.h"


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


/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


// typedef struct FCIO_LOG_MSG_T *FCIO_LOG_MSG;
// typedef void (*FCIO_LOG_CALLBACK)(FCIO_LOG_MSG msg);
// struct FCIO_LOG_MSG_T {
//   int   lvl;
//   int   line;
//   char *timestamp;
//   char *func;
//   char *msg;
// };


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


static bool fcio_log_did_init = FALSE;

static thread_t thread;

static mutex_t  msg_mutex  = mutex_init_static;
static cond_t   msg_cond   = cond_init_static;
static QUEUE    msg_queue  = NULL;

static mutex_t  cb_mutex = mutex_init_static;
static CVEC     cb_vec   = NULL;

static int fn_max_width = 40;

static const char *fcio_log_lvl_str_array[] = {
  "INFO",
  "WARN",
  "ERR"
};


/* ---------------------------------------------------------- Static function's ---------------------------------------------------------- */


static FCIO_LOG_MSG fcio_log_msg_create(int lvl, int line, char *timestamp, char *func, char *msg) {
  FCIO_LOG_MSG ret = xmalloc(sizeof(*ret));
  ret->lvl       = lvl;
  ret->line      = line;
  ret->timestamp = timestamp;
  ret->func      = func;
  ret->msg       = msg;
  return ret;
}

static void fcio_log_msg_free(FCIO_LOG_MSG msg) {
  ASSERT(msg);
  free(msg->timestamp);
  free(msg->func);
  free(msg);
}

static void fcio_log_run_callbacks(FCIO_LOG_MSG msg) {
  ASSERT(msg);
  FCIO_LOG_CALLBACK cb;
  size_t i = 0;
  while (TRUE) {
    mutex_lock(&cb_mutex);
    if (i >= new_cvec_size(cb_vec)) {
      mutex_unlock(&cb_mutex);
      break;
    }
    cb = (FCIO_LOG_CALLBACK)new_cvec_get(cb_vec, i++);
    mutex_unlock(&cb_mutex);
    cb(msg);
  }
}

_NO_RETURN
static void *fcio_log_thread_work(void *_UNUSED arg) {
  FCIO_LOG_MSG msg;
  while (TRUE) {
    mutex_lock(&msg_mutex);
    while (!queue_size(msg_queue)) {
      cond_wait(&msg_cond, &msg_mutex);
    }
    msg = queue_pop_front(msg_queue);
    mutex_unlock(&msg_mutex);
    fcio_log_run_callbacks(msg);
    fcio_log_msg_free(msg);
  }
}

static char *fcio_log_fn_str(const char *fn, size_t len, size_t max) {
  ASSERT(fn);
  if (len <= max) {
    return fmtstr("%*s", (int)max, fn);
  }
  else if (max > 3) {
    return fmtstr("...%s", (fn + ((len - max) + SLTLEN("..."))));
  }
  else {
    return fmtstr("%.*s", (int)max, "...");
  }
}

void fcio_log_enqueue_msg(int lvl, int line, const char *fn, size_t fn_len, const char *fmt, ...) {
  ASSERT(fn);
  ASSERT(fmt);
  FCIO_LOG_MSG log_msg;
  char *msg;
  va_list ap;
  /* We should probebly die here, as we have no other real way to inform the user they must init. */
  if (!fcio_log_did_init) {
    return;
  }
  va_start(ap, fmt);
  msg = valstr(fmt, ap, NULL);
  va_end(ap);
  log_msg = fcio_log_msg_create(lvl, line, COPY_OF(""), fcio_log_fn_str(fn, fn_len, fn_max_width), msg);
  MUTEX_ACTION(&msg_mutex,
    queue_push(msg_queue, log_msg);
  );
  cond_signal(&msg_cond);
}

void fcio_log_add_callback(FCIO_LOG_CALLBACK cb) {
  ASSERT(cb);
  /* We should probebly die here, as we have no other real way to inform the user they must init. */
  if (!fcio_log_did_init) {
    return;
  }
  MUTEX_ACTION(&cb_mutex,
    new_cvec_push_back(cb_vec, (void *)cb);
  );
}

const char *fcio_log_lvl_str(int lvl) {
  if (lvl >= 0 && LT(lvl, ARRAY_SIZE(fcio_log_lvl_str_array))) {
    return fcio_log_lvl_str_array[lvl];
  }
  else {
    return "";
  }
}

int fcio_log_lvl_str_max_width(void) {
  return 5;
}

int fcio_log_line_max_width(void) {
  return 5;
}

void fcio_log_init(void) {
  ASSERT(!msg_queue);
  ASSERT(!cb_vec);
  msg_queue = queue_create();
  cb_vec    = new_cvec_create();
  thread_create(&thread, NULL, fcio_log_thread_work, NULL);
  thread_detach(thread);
  fcio_log_did_init = TRUE;
}


/* ----------------------------- Fcio log va ----------------------------- */

_PRINTFLIKE(4, 0)
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
      MUTEX_ACTION(&fcio_log_mutex,
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

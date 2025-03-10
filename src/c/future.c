/** @file future.c

  @author  Melwin Svensson.
  @date    10-3-2025.

 */
#include "../include/proto.h"



/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


/* Opaque future structure for threaded operations. */
struct Future {
  mutex_t mutex;   /* Mutex to protect setting `result` and `ready`. */
  cond_t  cond;    /* Condition to signal when `result` is ready. */
  bool    ready;   /* Flag to tell potention listeners (or checkers) that the data is ready to be read. */
  void   *result;  /* Ptr to the data. */
};

typedef struct {
  Future *future;
  void *(*task)(void *);
  void *arg;
} FutureTask;


/* ---------------------------------------------------------- Function's ---------------------------------------------------------- */


/* Create a allocated `Future` structure. */
static Future *future_create(void) {
  Future *future = xmalloc(sizeof(*future));
  mutex_init(&future->mutex, NULL);
  cond_init(&future->cond, NULL);
  future->ready  = FALSE;
  future->result = NULL;
  return future;
}

/* Function that runs the future task and set the result and the ready flag, then signals the cond. */
static void *future_task_callback(void *arg) {
  ASSERT(arg);
  FutureTask *data = arg;
  void *result = data->task(data->arg);
  mutex_action(&data->future->mutex,
    data->future->result = result;
    data->future->ready = TRUE;
    cond_signal(&data->future->cond);
  );
  free(data);
  return NULL;
}

/* Free's a allocated `Future` structure. */
void future_free(Future *future) {
  ASSERT(future);
  mutex_destroy(&future->mutex);
  cond_destroy(&future->cond);
  free(future);
}

/* Get the result of the task related to `future`.  Note that this function is `blocking`. */
void *future_get(Future *future) {
  ASSERT(future);
  void *result;
  mutex_action(&future->mutex,
    while (!future->ready) {
      cond_wait(&future->cond, &future->mutex);
    }
    result = future->result;
  );
  return result;
}

/* Try to get the result of the task related to `future`.  Note that this function is
 * `non-blocking` and return's `TRUE` when the result is ready and could be retrieved. */
bool future_try_get(Future *future, void **result) {
  ASSERT(future);
  ASSERT(result);
  mutex_action(&future->mutex,
    *result = (future->ready ? future->result : NULL);
  );
  return (*result);
}

/* Create a future for `task` with `arg` passed to the `task`.  Return's the
 * `future`, use `future_get()` or `future_try_get()` to get the result of `task`. */
Future *future_submit(void *(*task)(void *), void *arg) {
  thread_t thread;
  FutureTask *data = xmalloc(sizeof(*data));
  data->future = future_create();
  data->task   = task;
  data->arg    = arg;
  thread_create(&thread, NULL, future_task_callback, data);
  thread_detach(thread);
  return data->future;
}

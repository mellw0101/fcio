/** @file queue.c

  @author  Melwin Svensson.
  @date    17-4-2025.

 */
#define _USE_ALL_BUILTINS
#include "../include/proto.h"


/* ---------------------------------------------------------- Define's ---------------------------------------------------------- */


#define QUEUE_DEFAULT_CAP  (8)
#define ASSERT_QUEUE(x)  \
  DO_WHILE(              \
    ASSERT((x));         \
    ASSERT((x)->data);   \
  )


/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


struct QUEUE_T {
  Ulong start;
  Ulong size;
  Ulong cap;
  void **data;
  void (*free_func)(void *);
};


/* ---------------------------------------------------------- Static function's ---------------------------------------------------------- */


/* Becomes a `nop` if there is no `free_func` set for the queue. */
static void queue_free_data(QUEUE q) {
  ASSERT_QUEUE(q);
  if (q->free_func) {
    for (Ulong i=q->start, end=(q->start + q->size) ; i<end; ++i) {
      q->free_func(q->data[i]);
    }
  }
}


/* ---------------------------------------------------------- Global function's ---------------------------------------------------------- */


QUEUE queue_create(void) {
  QUEUE q = xmalloc(sizeof *q);
  q->start     = 0;
  q->size      = 0;
  q->cap       = QUEUE_DEFAULT_CAP;
  q->data      = xmalloc(_PTRSIZE * q->cap);
  q->free_func = NULL;
  return q;
}

void queue_free(QUEUE q) {
  /* As a non fully internaly controlled container, make this a no-op on NULL. */
  if (!q) {
    return;
  }
  queue_free_data(q);
  free(q->data);
  free(q);
}

void queue_set_free_func(QUEUE q, void (*free_func)(void *)) {
  ASSERT(q);
  q->free_func = free_func;
}

Ulong queue_size(QUEUE q) {
  ASSERT_QUEUE(q);
  return q->size;
}

/* Consumes the first entry, and if the queue has `free_func` set also call it on the entry. */
void queue_pop(QUEUE q) {
  ASSERT(q);
  ALWAYS_ASSERT(q->size > 0);
  CALL_IF_VALID(q->free_func, q->data[q->start]);
  if (q->size == 1) {
    q->size  = 0;
    q->start = 0;
  }
  else {
    --q->size;
    ++q->start;
  }
}

void queue_push(QUEUE q, void *data) {
  ASSERT_QUEUE(q);
  ASSERT(data);
  if ((q->start + q->size) == q->cap) {
    /* If the queue is full. */
    if (!q->start) {
      q->cap *= 2;
      q->data = xrealloc(q->data, (_PTRSIZE * q->cap));
    }
    else {
      memmove(q->data, (q->data + q->start), (_PTRSIZE * (q->cap - q->start)));
      q->start = 0;
    }
  }
  q->data[q->start + q->size++] = data;
}

/* Must be called on a non empty queue. */
void *queue_front(QUEUE q) {
  ASSERT(q);
  ALWAYS_ASSERT(q->size > 0);
  return q->data[q->start];
}

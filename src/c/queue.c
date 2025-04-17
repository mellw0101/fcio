/** @file queue.c

  @author  Melwin Svensson.
  @date    17-4-2025.

 */
#include "../include/proto.h"


/* ---------------------------------------------------------- Forward decl's ---------------------------------------------------------- */


typedef struct QueueNode  QueueNode;


/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


struct QueueNode {
  void *data;
  QueueNode *next;
};

struct Queue {
  QueueNode *head;
  QueueNode *tail;
  Ulong size;
  FreeFuncPtr free_func;
};


/* ---------------------------------------------------------- Functions's ---------------------------------------------------------- */


Queue *queue_create(void) {
  Queue *q = xmalloc(sizeof(*q));
  q->head = NULL;
  q->tail = NULL;
  q->size = 0;
  q->free_func = NULL;
  return q;
}

void queue_free(Queue *q) {
  QueueNode *temp;
  /* Make this function a `NO-OP` function. */
  if (!q) {
    return;
  }
  while (q->head) {
    temp = q->head;
    q->head = q->head->next;
    CALL_IF_VALID(q->free_func, temp->data);
    free(temp);
  }
  free(q);
}

void queue_set_free_func(Queue *q, FreeFuncPtr free_func) {
  ASSERT(q);
  q->free_func = free_func;
}

void queue_enqueue(Queue *q, void *data) {
  ASSERT(q);
  ASSERT(data);
  QueueNode *node = xmalloc(sizeof(*node));
  node->data = data;
  node->next = NULL;
  if (!q->tail) {
    q->head = node;
    q->tail = node;
  }
  else {
    q->tail->next = node;
    q->tail = node;
  }
  ++q->size;
}

void *queue_pop(Queue *q) {
  ASSERT(q);
  QueueNode *node = q->head;
  void *data;
  if (!node) {
    return NULL;
  }
  data = node->data;
  /* Advance the head to the next entry. */
  q->head = q->head->next;
  if (!q->head) {
    q->tail = NULL;
  }
  free(node);
  --q->size;
  return data;
}

void *queue_peak(Queue *q) {
  ASSERT(q);
  if (!q->head) {
    return NULL;
  }
  return q->head->data;
}

Ulong queue_size(Queue *q) {
  return q->size;
}

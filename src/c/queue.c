/** @file queue.c

  @author  Melwin Svensson.
  @date    17-4-2025.

 */
#include "../include/proto.h"


/* ---------------------------------------------------------- Define's ---------------------------------------------------------- */


#define QUEUE_DEFAULT_CAP  (8)
#define ASSERT_QUEUE(x)  \
  DO_WHILE(              \
    ASSERT((x));         \
    ASSERT((x)->data);   \
  )


/* ---------------------------------------------------------- Typedef's ---------------------------------------------------------- */



// typedef struct QueueNode  QueueNode;


/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


struct QUEUE_T {
  Ulong start;
  Ulong size;
  Ulong cap;
  void **data;
  void (*free_func)(void *);
};

// struct QueueNode {
//   void *data;
//   QueueNode *next;
// };

// struct Queue {
//   QueueNode *head;
//   QueueNode *tail;
//   Ulong size;
//   FreeFuncPtr free_func;
//   mutex_t mutex;
// };


/* ---------------------------------------------------------- Static function's ---------------------------------------------------------- */


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
  FREE(q->data);
  FREE(q);
}

void queue_set_free_func(QUEUE q, void (*free_func)(void *)) {
  ASSERT(q);
  q->free_func = free_func;
}

Ulong queue_size(QUEUE q) {
  ASSERT_QUEUE(q);
  return q->size;
}

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
      MEMMOVE(q->data, (q->data + q->start), (_PTRSIZE * (q->cap - q->start)));
      q->start = 0;
    }
  }
  q->data[q->start + q->size++] = data;
}

void *queue_front(QUEUE q) {
  ASSERT(q);
  ALWAYS_ASSERT(q->size > 0);
  return q->data[q->start];
}

// Queue *queue_create(void) {
//   Queue *q = xmalloc(sizeof(*q));
//   q->head = NULL;
//   q->tail = NULL;
//   q->size = 0;
//   q->free_func = NULL;
//   mutex_init(&q->mutex, NULL);
//   return q;
// }

// void queue_free(Queue *q) {
//   QueueNode *temp;
//   /* Make this function a `NO-OP` function. */
//   if (!q) {
//     return;
//   }
//   mutex_action(&q->mutex,
//     while (q->head) {
//       temp = q->head;
//       q->head = q->head->next;
//       CALL_IF_VALID(q->free_func, temp->data);
//       free(temp);
//     }
//   );
//   mutex_destroy(&q->mutex);
//   free(q);
// }

// void queue_set_free_func(Queue *q, FreeFuncPtr free_func) {
//   ASSERT(q);
//   mutex_action(&q->mutex,
//     q->free_func = free_func;
//   );
// }

// void queue_enqueue(Queue *q, void *data) {
//   ASSERT(q);
//   ASSERT(data);
//   QueueNode *node = xmalloc(sizeof(*node));
//   node->data = data;
//   node->next = NULL;
//   mutex_action(&q->mutex,
//     if (!q->tail) {
//       q->head = node;
//       q->tail = node;
//     }
//     else {
//       q->tail->next = node;
//       q->tail = node;
//     }
//     ++q->size;
//   );
// }

// void *queue_pop(Queue *q) {
//   ASSERT(q);
//   QueueNode *node;
//   void *data;
//   mutex_action(&q->mutex,
//     if (!q->head) {
//       return NULL;
//     }
//     node = q->head;
//     data = node->data;
//     /* Advance the head to the next entry. */
//     q->head = q->head->next;
//     if (!q->head) {
//       q->tail = NULL;
//     }
//     free(node);
//     --q->size;
//   );
//   return data;
// }

// void *queue_peak(Queue *q) {
//   ASSERT(q);
//   void *ret;
//   mutex_action(&q->mutex,
//     if (!q->head) {
//       ret = NULL;
//     }
//     else {
//       ret = q->head->data;
//     }
//   );
//   return ret;
// }

// Ulong queue_size(Queue *q) {
//   ASSERT(q);
//   Ulong ret;
//   mutex_action(&q->mutex,
//     ret = q->size;
//   );
//   return ret;
// }

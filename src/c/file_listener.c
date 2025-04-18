/** @file file_listener.c

  @author  Melwin Svensson.
  @date    17-4-2025.

 */
#include "../include/proto.h"


#define EVENT_SIZE    			(sizeof(struct inotify_event))
#define EVENT_BUF_LEN 			(1024 * (EVENT_SIZE + 16))


/* ---------------------------------------------------------- Typedef's ---------------------------------------------------------- */


typedef struct FcioFileListener  FcioFileListener;
typedef struct FileListenerNode  FileListenerNode;
typedef void (*FileListenerCb)   (int);


/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


typedef struct {
  FileListenerCb callback;
  Uint mask;
} QueueEvent;

struct FileListenerNode {
  char *file;
  bool running;
  thread_t thread;
  int fd;
  int wd;
  FileListenerCb callback;
  FcioFileListener *listener;
};

struct FcioFileListener {
  HashMap *files;
  Queue *queue;
  mutex_t mutex;
  cond_t cond;
  thread_t thread;
  bool running;
};


/* ---------------------------------------------------------- Function's ---------------------------------------------------------- */


/* ----------------------------- QueueEvent ----------------------------- */

_UNUSED static QueueEvent *queue_event_create(FileListenerCb callback, Uint mask) {
  ASSERT(callback);
  ASSERT(mask);
  QueueEvent *event = xmalloc(sizeof(*event));
  event->callback = callback;
  event->mask = mask;
  return event;
}

/* ----------------------------- FileListenerNode ----------------------------- */

_UNUSED static void *filelistener_node_task(void *arg) {
  ASSERT(arg);
  FileListenerNode *node = arg;
  char buffer[EVENT_BUF_LEN];
  long len;
  struct inotify_event *event;
  while (node->running) {
    len = read(node->fd, buffer, EVENT_BUF_LEN);
    if (len == -1) {
      writef("%s: Error reading fd\n", __func__);
      break;
    }
    for (long i=0; i<len;) {
      event = (struct inotify_event *)&buffer[i];
      mutex_action(&node->listener->mutex,
        queue_enqueue(node->listener->queue, queue_event_create(node->callback, event->mask));
        cond_signal(&node->listener->cond);
      );
      i += (EVENT_SIZE + event->len);
    }
  }
  return NULL;
}

_UNUSED static FileListenerNode *filelistener_node_create(const char *const restrict file, Uint mask, FileListenerCb callback, FcioFileListener *const listener) {
  FileListenerNode *node = xmalloc(sizeof(*node));
  node->file = copy_of(file);
  node->callback = callback;
  node->running = TRUE;
  node->listener = listener;
  ALWAYS_ASSERT((node->fd = inotify_init()) >= 0);
  ALWAYS_ASSERT((node->wd = inotify_add_watch(node->fd, node->file, mask)) >= 0);
  ALWAYS_ASSERT(thread_create(&node->thread, NULL, filelistener_node_task, node) == 0);
  return node;
}

/* ----------------------------- FileListener ----------------------------- */

_UNUSED static void *filelistener_task(void *arg) {
  FcioFileListener *listener = arg;
  QueueEvent *event;
  while (TRUE) {
    mutex_action(&listener->mutex,
      while (!queue_size(listener->queue) && listener->running) {
        cond_wait(&listener->cond, &listener->mutex);
      }
      if (!listener->running) {
        mutex_unlock(&listener->mutex);
        break;
      }
      event = queue_pop(listener->queue);
    );
    if (event) {
      event->callback(event->mask);
      free(event);
      event = NULL;
    }
  }
  return NULL;
}

// FcioFileListener *filelistener_create(void) {
//   FcioFileListener *listener = xmalloc(sizeof(*listener));
//   listener->files = hashmap_create();
//   listener->queue = queue_create();
//   queue_set_free_func(listener->queue, free);
//   mutex_init(&listener->mutex, NULL);
//   cond_init(&listener->cond, NULL);
//   listener->running = TRUE;
//   ALWAYS_ASSERT(thread_create(&listener->thread, NULL, filelistener_task, listener) == 0);
//   return listener;
// }

// void filelistener_add_file(FcioFileListener *const listener, const char *const restrict file, Uint mask, FileListenerCb callback) {
//   ASSERT(file);
//   ASSERT(callback);
//   FileListenerNode *node = hashmap_get(listener->files, file);
//   if (node) {
//     return;
//   }
//   node = filelistener_node_create(file, mask, callback, listener);
//   hashmap_insert(listener->files, file, node);
// }

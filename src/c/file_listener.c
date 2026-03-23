/** @file file_listener.c

  @author  Melwin Svensson.
  @date    17-4-2025.

 */
#define _USE_ALL_BUILTINS
#include "../include/proto.h"


#if !__WIN__


#define EVENT_SIZE    			(sizeof(struct inotify_event))
#define EVENT_BUF_LEN 			(1024 * (EVENT_SIZE + 16))

#define SENTINAL_RM      (-1)
#define SENTINAL_EV      (-2)
#define SENTINAL_UPDATE  (-3)


/* ---------------------------------------------------------- Typedef's ---------------------------------------------------------- */


typedef struct FILE_LISTENER_EVENT_T  *FILE_LISTENER_EVENT;
typedef struct FILE_LISTENER_NODE_T   *FILE_LISTENER_NODE;
typedef struct FILE_LISTENER_EPOLL_T  *FILE_LISTENER_EPOLL;


/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


struct FILE_LISTENER_EVENT_T {
  void *data;
  Uint mask;
  FILE_LISTENER_CB callback;
};

struct FILE_LISTENER_NODE_T {
  int fd;
  int wd;
  char *file;
  void *data;
  FILE_LISTENER_CB callback;
};

struct FILE_LISTENER_T {
  HNMAP    wfds;
  HMAP_PH  nodes;
  QUEUE    queue;
  mutex_t  mutex;
  cond_t   cond;
  bool     running_cb;
  bool     running_epoll;
  int      ifd;
  /* The epoll file-descriptior. */
  int      efd;
  /* The sentinel kill file-descriptior. */
  int      kfd;
  thread_t thread_cb;
  thread_t thread_epoll;
  FILE_LISTENER_NODE sentinal;
};


/* ---------------------------------------------------------- Static function's ---------------------------------------------------------- */


/* Add `fd` with `ptr` as its data for the epoll fd `efd`. */
static void epoll_add_fd(int efd, int fd, void *ptr) {
  struct epoll_event ev;
  ev.events   = EPOLLIN;
  ev.data.ptr = ptr;
  if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) < 0) {
    die_callback("Failed when calling 'epoll_ctl': %s\n", strerror(errno));
  }
}

/* ----------------------------- FILE_LISTENER ----------------------------- */

/* The only way an callback event should be enqueued, as this is mutex protected. */
static void file_listener_enqueue_callback(FILE_LISTENER fl, FILE_LISTENER_CB callback, void *data, Uint mask) {
  ASSERT(fl);
  ASSERT(callback);
  FILE_LISTENER_EVENT ev = xmalloc(sizeof(*ev));
  ev->callback = callback;
  ev->data     = data;
  ev->mask     = mask;
  /* Push the event into the file-listener's queue. */
  MUTEX_ACTION(&fl->mutex,
    queue_push(fl->queue, ev);
    cond_signal(&fl->cond);
  );
}

/* The only way a node should be removed, as this ensures the epoll thread itself handles all cleanup. */
static void file_listener_add_rm_node(FILE_LISTENER fl, FILE_LISTENER_NODE node) {
  ASSERT(fl);
  ASSERT(node);
  Ulong val = 1;
  /* Construct the removal node. */
  FILE_LISTENER_NODE rnode = xmalloc(sizeof(*rnode));
  rnode->fd = eventfd(0, 0);
  /* Set our sentinal. */
  rnode->wd = SENTINAL_RM;
  /* Here we store the node to be removed as the data of our
   * sentinal, this way the epoll thread will handle all cleanup. */
  rnode->data = node;
  epoll_add_fd(fl->efd, rnode->fd, rnode);
  /* Send  */
  write(rnode->fd, &val, sizeof(val));
}

static void file_listener_add_update_node(FILE_LISTENER fl, FILE_LISTENER_NODE node, FILE_LISTENER_EVENT data) {
  ASSERT(fl);
  ASSERT(node);
  ASSERT(data);
  Ulong val = 1;
  /* Construct the update node. */
  FILE_LISTENER_NODE unode = xmalloc(sizeof(*unode));
  unode->fd = eventfd(0, 0);
  /* Set the sentinal. */
  unode->wd = SENTINAL_UPDATE;
  unode->data     = node;
  /* Store the update data in the callback. */
  unode->callback = (FILE_LISTENER_CB)data;
  epoll_add_fd(fl->efd, unode->fd, unode);
  write(unode->fd, &val, sizeof(val));
}

/* The callback used to terminate the `cb-thread-loop`.  This way the loop will terminate itself. */
static void file_listener_cb_kill_sentinal(FILE_LISTENER fl, Uint _UNUSED mask) {
  ASSERT(fl);
  fl->running_cb = FALSE;
}

/* The callback our `nodes` hashmap uses when removing a node and on freeing itself. */
static void file_listener_node_free(FILE_LISTENER_NODE node) {
  ASSERT(node);
  inotify_rm_watch(node->fd, node->wd);
  free(node->file);
  free(node);
}

/* ----------------------------- Shutdown ----------------------------- */

static void file_listener_shutdown_epoll(FILE_LISTENER fl) {
  ASSERT(fl);
  Ulong val = 1;
  write(fl->kfd, &val, sizeof(val));
  pthread_join(fl->thread_epoll, NULL);
}

static void file_listener_shutdown_cb(FILE_LISTENER fl) {
  ASSERT(fl);
  file_listener_enqueue_callback(fl, (FILE_LISTENER_CB)file_listener_cb_kill_sentinal, fl, 0);
  pthread_join(fl->thread_cb, NULL);
}

/* ----------------------------- Thread-Loop's ----------------------------- */

/* The main listener's loop, this simply processes enqueued callbacks made by all nodes. */
static void *file_listener_thread_loop_cb(FILE_LISTENER fl) {
  ASSERT(fl);
  FILE_LISTENER_EVENT ev;
  while (fl->running_cb) {
    mutex_lock(&fl->mutex);
    /* TODO: We should not need these running checks, as the only way its set to false is via a callback... */
    while (!queue_size(fl->queue) && fl->running_cb) {
      cond_wait(&fl->cond, &fl->mutex);
    }
    if (!fl->running_cb) {
      mutex_unlock(&fl->mutex);
      break;
    }
    ev = queue_front(fl->queue);
    queue_pop(fl->queue);
    mutex_unlock(&fl->mutex);
    /* Process the event. */
    ev->callback(ev->data, ev->mask);
    free(ev);
  }
  return NULL;
}

/* The epoll-thread's work loop, here all events only enqueue its given callback for the main loop to process. */
static void *file_listener_thread_loop_epoll(FILE_LISTENER fl) {
  struct epoll_event events[64];
  int n;
  FILE_LISTENER_EVENT update_data;
  FILE_LISTENER_NODE node;
  FILE_LISTENER_NODE wnode;
  char buf[EVENT_BUF_LEN];
  long len;
  struct inotify_event *ev;
  while (fl->running_epoll) {
    n = epoll_wait(fl->efd, events, 64, -1);
    for (int i=0; i<n; ++i) {
      node = events[i].data.ptr;
      /* NULL-Sentinel, used for termination. */
      if (!node) {
        fl->running_epoll = FALSE;
        break;
      }
      /* Node removal sentinal. */
      else if (node->wd == SENTINAL_RM) {
        epoll_ctl(fl->efd, EPOLL_CTL_DEL, node->fd, NULL);
        close(node->fd);
        hnmap_remove(fl->wfds, ((FILE_LISTENER_NODE)node->data)->wd);
        hmap_ph_remove(fl->nodes, ((FILE_LISTENER_NODE)node->data)->file);
        free(node);
      }
      else if (node->wd == SENTINAL_UPDATE) {
        epoll_ctl(fl->efd, EPOLL_CTL_DEL, node->fd, NULL);
        close(node->fd);
        wnode = node->data;
        update_data = (FILE_LISTENER_EVENT)node->callback;
        /* Update the wd. */
        hnmap_remove(fl->wfds, wnode->wd);
        inotify_rm_watch(fl->ifd, wnode->wd);
        wnode->wd = inotify_add_watch(fl->ifd, wnode->file, update_data->mask);
        hnmap_insert(fl->wfds, wnode->wd, wnode);
        /* Update the callback and data. */
        wnode->data     = update_data->data;
        wnode->callback = update_data->callback;
        free(update_data);
        free(node);
      }
      /* Event sentinal. */
      else if (node->wd == SENTINAL_EV) {
        len = read(fl->ifd, buf, EVENT_BUF_LEN);
        if (len < 0) {
          die_callback("Failed to read inotify fd.\n");
        }
        for (long ei=0; ei<len;) {
          ev = (struct inotify_event *)&buf[ei];
          if ((wnode = hnmap_get(fl->wfds, ev->wd))) {
            file_listener_enqueue_callback(fl, wnode->callback, wnode->data, ev->mask);
          }
          ei += (EVENT_SIZE + ev->len);
        }
      }
      /* TODO: We should always terminate here, as this should never happen. */
    }
  }
  return NULL;
}

/* Called after all init has been done, this starts both threads. */
static void file_listener_run(FILE_LISTENER fl) {
  ASSERT(fl);
  fl->running_cb    = TRUE;
  fl->running_epoll = TRUE;
  if (thread_create(&fl->thread_cb, NULL, (void *(*)(void *))file_listener_thread_loop_cb, fl) != 0) {
    /* TODO: Here we should have a single function to call to cleanup. */
    die_callback("Failed to create FILE_LISTENER callback thread.\n");
  }
  if (thread_create(&fl->thread_epoll, NULL, (void *(*)(void *))file_listener_thread_loop_epoll, fl) != 0) {
    /* TODO: ~ */
    thread_cancel(fl->thread_cb);
    die_callback("Failed to create FILE_LISTENER epoll thread.\n");
  }
}

/* ----------------------------- Init ----------------------------- */

static void file_listener_init_epoll(FILE_LISTENER fl) {
  ASSERT(fl);
  if ((fl->efd = epoll_create1(0)) < 0) {
    die_callback("Failed when calling 'epoll_create1': %s\n", strerror(errno));
  }
  /* Create our shutdown sentinal. */
  fl->kfd = eventfd(0, 0);
  epoll_add_fd(fl->efd, fl->kfd, NULL);
  /* Create our event sentinal. */
  fl->sentinal     = xmalloc(sizeof(*fl->sentinal));
  fl->sentinal->wd = SENTINAL_EV;
  epoll_add_fd(fl->efd, fl->ifd, fl->sentinal);
}


/* ---------------------------------------------------------- Global function's ---------------------------------------------------------- */


FILE_LISTENER file_listener_create(void) {
  FILE_LISTENER fl = xmalloc(sizeof(*fl));
  fl->wfds  = hnmap_create();
  fl->nodes = hmap_ph_create();
  hmap_ph_set_free_func(fl->nodes, (void (*)(void *))file_listener_node_free);
  fl->queue = queue_create();
  mutex_init(&fl->mutex, NULL);
  cond_init(&fl->cond, NULL);
  if ((fl->ifd = inotify_init1(IN_NONBLOCK)) < 0) {
    die_callback("Failed when calling 'inotify_init1': %s\n", strerror(errno));
  }
  file_listener_init_epoll(fl);
  file_listener_run(fl);
  return fl;
}

void file_listener_kill(FILE_LISTENER fl) {
  ASSERT(fl);
  /* Kill both thread-loop's. */
  file_listener_shutdown_epoll(fl);
  file_listener_shutdown_cb(fl);
  mutex_destroy(&fl->mutex);
  cond_destroy(&fl->cond);
  hnmap_free(fl->wfds);
  hmap_ph_free(fl->nodes);
  queue_free(fl->queue);
  /* Close both epoll fd's. */
  close(fl->efd);
  close(fl->kfd);
  /* Now close the inotify fd. */
  close(fl->ifd);
  free(fl->sentinal);
  free(fl);
}

void file_listener_add_file(FILE_LISTENER fl,
  const char *const restrict file, FILE_LISTENER_CB cb, void *data, Uint mask)
{
  ASSERT(fl);
  ASSERT(file);
  ASSERT(cb);
  FILE_LISTENER_NODE node;
  /* Ensure we only add one listener per file, and that the file exists. */
  if (!file_exists(file) || hmap_ph_get(fl->nodes, file)) {
    return;
  }
  node           = xmalloc(sizeof(*node));
  node->file     = copy_of(file);
  node->callback = cb;
  node->data     = data;
  node->fd       = fl->ifd;
  if ((node->wd = inotify_add_watch(node->fd, node->file, mask)) < 0) {
    die_callback("Failed when calling 'inotify_add_watch' for file: '%s': %s\n", file, strerror(errno));
  }
  hnmap_insert(fl->wfds, node->wd, node);
  hmap_ph_insert(fl->nodes, file, node);
}

void file_listener_rm_file(FILE_LISTENER fl, const char *const restrict file) {
  ASSERT(fl);
  ASSERT(file);
  FILE_LISTENER_NODE node = hmap_ph_get(fl->nodes, file);
  if (node) {
    file_listener_add_rm_node(fl, node);
  }
}

void file_listener_update_file_callback(FILE_LISTENER fl,
  const char *const restrict file, FILE_LISTENER_CB cb, void *data, Uint mask)
{
  ASSERT(fl);
  ASSERT(file);
  ASSERT(cb);
  FILE_LISTENER_EVENT ev;
  FILE_LISTENER_NODE node = hmap_ph_get(fl->nodes, file);
  if (node) {
    ev = xmalloc(sizeof(*ev));
    ev->callback = cb;
    ev->data = data;
    ev->mask = mask;
    file_listener_add_update_node(fl, node, ev);
  }
}

#endif

/** @file atomicbool.c

  @author  Melwin Svensson.
  @date    7-4-2025.

 */
#include "../include/proto.h"

#if !__WIN__

struct atomicbool {
  mutex_t mutex;
  int value;
};


atomicbool *atomicbool_create(void) {
  atomicbool *ab = xmalloc(sizeof(*ab));
  mutex_init(&ab->mutex, NULL);
  ab->value = FALSE;
  return ab;
}

void atomicbool_free(atomicbool *ab) {
  if (!ab) {
    return;
  }
  mutex_destroy(&ab->mutex);
  free(ab);
}

bool atomicbool_get(atomicbool *ab) {
  ASSERT(ab);
  bool ret;
  mutex_action(&ab->mutex,
    ret = ab->value;
  );
  return ret;
}

void atomicbool_set_true(atomicbool *ab) {
  ASSERT(ab);
  mutex_action(&ab->mutex,
    ab->value = TRUE;
  );
}

void atomicbool_set_false(atomicbool *ab) {
  ASSERT(ab);
  mutex_action(&ab->mutex,
    ab->value = FALSE;
  );
}

#endif
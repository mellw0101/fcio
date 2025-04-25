/** @file atomic_bool_sync.c

  @author  Melwin Svensson.
  @date    6-4-2025.

 */
#include "../include/proto.h"


int atomic_bool_sync_get(atomic_bool_sync *const b) {
  return __sync_fetch_and_add(&b->value, 0);
}

void atomic_bool_sync_set_true(atomic_bool_sync *const b) {
  __sync_lock_test_and_set(&b->value, TRUE);
}

void atomic_bool_sync_set_false(atomic_bool_sync *const b) {
  __sync_lock_test_and_set(&b->value, FALSE);
}

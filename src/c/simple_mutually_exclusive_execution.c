/** @file simple_mutually_exclusive_execution.c
 * 
 * @author Melwin Svensson.
 *
 * This is the core functionality for my mutual execution algo. 
 */
#include "../include/proto.h"


/* ---------------------------------------------------------- Define's ---------------------------------------------------------- */


#ifndef __WORDSIZE
# error "Unknown arch bit size, to resolve this error, define '__WORDSIZE', please note that non 32/64 bit sizes might not work."
#endif
#define SMUTEX_UINT_T  PP_CAT(uint, __WORDSIZE)

#define _ALIGN(x)  __attribute__((__aligned__(x)))


/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


/* Simple mutual exclusive single winner variable. */
struct SMUTEX_SW_VAR_T {
  volatile SMUTEX_UINT_T gate;
  volatile SMUTEX_UINT_T proof;
} _ALIGN(sizeof(SMUTEX_UINT_T) * 2);

struct SMUTEX_T {
  /* Used to always derive a single mutally exclusive winner. */
  struct SMUTEX_SW_VAR_T var;
  /* This acts like the true key, in that only and only a true winner can
   * write to this, meaning that when any given thread is the only winner,
   * it will get its id returned, meaning it now holds the lock, and can thereform
   * set this holder before clering `var->gate`. */
  volatile SMUTEX_UINT_T holder;
};


/* ---------------------------------------------------------- Static function's ---------------------------------------------------------- */


static SMUTEX_UINT_T smutex_try_win(struct SMUTEX_SW_VAR_T *v, volatile SMUTEX_UINT_T id) {
  ASSERT(v);
  if (!v->gate) {
    /* To ensure that when there is no winner, we will automaticly end up with a cleared gate. we add to the gate. */
    v->gate  = (v->gate + id);
    v->proof = v->gate;
    /* We have a mutually exclusive winner. */
    if (v->gate == id && v->proof == id) {
      return id;
    }
    /* When we did not win, remove the id from the gate, this ensures that, in all cases
     * where no one won, the gate will be clear.  And be design will never be clear on a winner. */
    v->gate = (v->gate - id);
  }
  return 0;
}


/* ---------------------------------------------------------- Global function's ---------------------------------------------------------- */


/* Returns and allocated simple-mutex.  Note that this can be released using `free()`. */
SMUTEX smutex_create(void) {
  SMUTEX sm = xmalloc(sizeof(*sm));
  sm->var.gate = 0;
  sm->holder   = 0;
  return sm;
}

void smutex_lock(SMUTEX sm) {
  ASSERT(sm);
  volatile SMUTEX_UINT_T attempt = 0;
  volatile SMUTEX_UINT_T id = pthread_self();
  while (1) {
    /* Becuase this can only ever be written to by the holder of a already locked mutex, we can
     * ensure that even if a thread reads this just as the lock is won, retrying will not matter. */
    if (!sm->holder) {
      /* We now hold the lock. */
      if (smutex_try_win(&sm->var, id) == id) {
        sm->holder = id;
        return;
      }
    }
    /* TODO: The best way to scale the wait-time between each try should be n^2, as we should always be close enough.
     * and very performant always, as each time we fail we simply sleep double that wait, which should on avarage,
     * taking into account wildy ranging possible waits, perform close enough, and it should also make the lock
     * itself better at not bunching all attempts at some set interval.  Currently this gives us 32 or 64 attempts
     * with increasingly long sleeps, for now we will just let it loop like this, as the lock holder must ensure they
     * unlock it.  I don't know any way i would want to create the ability for any non lock holder to do anything about
     * it, even when the holder has crashed or something else, as I think that it's mush better to ensure lock
     * exclusivity is final.  And that the responsibilty of ensuring the lock is unlocked is on the holder and only
     * the holder. */
    hiactime_nsleep(50 * (1 << (((attempt++ & (__WORDSIZE - 1)) + (id & (__WORDSIZE - 1))) & (__WORDSIZE - 1))));
  }
}

void smutex_unlock(SMUTEX sm) {
  ASSERT(sm);
  /* Because the only way to reach this is to truly hold the lock, as
   * in all other cases, the lock would by definition not be mutual,
   * we should never ever need to verify that the holder is the caller,
   * as the only way that can happen is incorrect use, or the lock not working.
   * and if the lock does not work, then it does not matter anyway.  Instead,
   * to provide some protection against missuse, we only clear the gate when
   * there is an actual holder.  And because the holder by definition is currently,
   * able to execute fully exclusivly, this should be the only way we actually
   * modify the gate. */
  if (sm->holder) {
    /* To ensure that the holder is always cleared after the gate is reset, we create a load dependency, so that
     * the holder is always the last to be cleared, and becuse the only ever true way this should ever be reached
     * is with the gate holding the absolute value of the holder, and just to prove to myself that this always is
     * true, we use the holder to clear the gate. */
    sm->var.gate = (sm->var.gate - sm->holder);
    sm->holder   = sm->var.gate;
  }
}

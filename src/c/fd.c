/** @file fd.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"


/* Lock a file descriptor. */
void fdlock(int fd, short type) {
  struct flock lock = {0};
  /* Set flock field's. */
  lock.l_type   = type;
  lock.l_whence = SEEK_SET;
  lock.l_pid    = getpid();
  /* Perform the locking. */
  ALWAYS_ASSERT(fcntl(fd, F_SETLKW, &lock) != -1);
}

/* Unlock a file desctiptor. */
void fdunlock(int fd) {
  struct flock lock = {0};
  /* Set flock fields. */
  lock.l_type   = F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_pid    = getpid();
  /* Perform the unlocking. */
  ALWAYS_ASSERT(fcntl(fd, F_SETLK, &lock) != -1);
}
